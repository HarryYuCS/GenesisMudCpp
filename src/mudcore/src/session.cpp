#include <mudcore/session.hpp>
#include <mudcore/display_line.hpp>
#include <mudcore/event_bus.hpp>
#include <mudcore/gmcp_parser.hpp>
#include <mudcore/inbound_pipeline.hpp>
#include <mudcore/outbound_pipeline.hpp>
#include <mudcore/game_state.hpp>
#include <mudcore/connection_life_cycle.hpp>

#include <network/telnet_codec.hpp>

#include <boost/asio/post.hpp>

#include <format>
#include <span>
#include <string_view>
#include <utility>

namespace genesis::mudcore {

namespace {

std::string formatDebugGmcpLine(const std::string_view rawBody) {
    constexpr std::size_t kMaxBodyPreview = 80;
    std::string line = "GMCP: ";
    const auto parsed = GmcpParser{}.parse(rawBody);
    if (!parsed.has_value()) {
        line += rawBody.size() > kMaxBodyPreview ? std::string(rawBody.substr(0, kMaxBodyPreview)) + "..."
                                                 : std::string(rawBody);
        return line;
    }

    line += parsed->package;
    if (!parsed->jsonBody.empty()) {
        line += ' ';
        if (parsed->jsonBody.size() > kMaxBodyPreview) {
            line += parsed->jsonBody.substr(0, kMaxBodyPreview);
            line += "...";
        } else {
            line += parsed->jsonBody;
        }
    }
    return line;
}

} // namespace

Session::Session(boost::asio::io_context& ioContext)
    : ioContext_(ioContext)
    , tcpSession_(ioContext)
    , connectionLifeCycle_([this](std::string_view body) { sendGmcp(body); })
{
    tcpSession_.setReadHandler([this](std::span<const std::byte> data) { onTcpRead(data); });
    tcpSession_.setConnectedHandler([this]() {
        eventBus_.enqueueInboundEvent(Event{EventType::Connected, std::monostate{}});
    });
    tcpSession_.setDisconnectedHandler([this]() {
        telnetCodec_.reset();
        eventBus_.enqueueInboundEvent(Event{EventType::Disconnected, std::monostate{}});
    });
    tcpSession_.setErrorHandler([this](const genesis::network::NetworkError& error) {
        eventBus_.enqueueInboundEvent(Event{EventType::Error, error});
    });
}

void Session::connect(const std::string& host, std::uint16_t port) {
    connectionLifeCycle_.onConnectRequested();
    boost::asio::post(ioContext_, [this, host, port]() {
        tcpSession_.connect(genesis::network::ConnectionConfig{host, port});
    });
}

void Session::disconnect() {
    connectionLifeCycle_.onTcpDisconnected();
    gameState_.reset();
    boost::asio::post(ioContext_, [this]() { tcpSession_.disconnect(); });
}

PollResult Session::poll() {
    PollResult result;

    for (Event& event : eventBus_.drainInboundEvents()) {
        switch (event.type) {
        case EventType::Connected:
            connectionLifeCycle_.onTcpConnected();
            result.lines.push_back(DisplayLine{OutputSink::System, "Connected."});
            break;
        case EventType::Disconnected:
            connectionLifeCycle_.onTcpDisconnected();
            gameState_.reset();
            result.lines.push_back(DisplayLine{OutputSink::System, "Disconnected."});
            break;
        case EventType::Error:
            if (const auto* error = std::get_if<genesis::network::NetworkError>(&event.payload)) {
                result.lines.push_back(
                    DisplayLine{OutputSink::System, "Network error: " + error->errorMessage});
            }
            break;
        case EventType::GmcpNegotiated:
            connectionLifeCycle_.onGmcpNegotiated();
            break;
        case EventType::MudText: {
            if (const auto* text = std::get_if<std::string>(&event.payload)) {
                if (auto line = inboundPipeline_.processMudText(*text)) {
                    result.lines.push_back(std::move(*line));
                }
            }
            break;
        }
        case EventType::GmcpRaw: {
            if (const auto* raw = std::get_if<std::string>(&event.payload)) {
                if (debugLogging_) {
                    result.lines.push_back(
                        DisplayLine{OutputSink::System, formatDebugGmcpLine(*raw)});
                }
                auto gmcpResult = inboundPipeline_.processGmcp(*raw, gameState_);
                if (gmcpResult.line.has_value()) {
                    result.lines.push_back(std::move(*gmcpResult.line));
                }
                if (gmcpResult.stateChanged) {
                    result.stateChanged = true;
                }
                if (gmcpResult.playerLoggedIn) {
                    connectionLifeCycle_.onPlayerLoggedIn();
                }
            }
            break;
        }
        }
    }

    result.connectionPhase = connectionLifeCycle_.phase();
    return result;
}

const GameState& Session::gameState() const noexcept {
    return gameState_;
}

ConnectionPhase Session::connectionPhase() const noexcept {
    return connectionLifeCycle_.phase();
}

void Session::sendCommand(std::string_view command) {
    if (auto line = outboundPipeline_.process(command)) {
        boost::asio::post(ioContext_, [this, line = std::move(*line)]() { sendLine(line); });
    }
}

void Session::sendGmcp(const std::string_view body) {
    boost::asio::post(ioContext_, [this, payload = std::string(body)]() {
        tcpSession_.send(telnetCodec_.encodeGmcp(payload));
    });
}

void Session::sendClientSize(const unsigned width, const unsigned height) {
    sendGmcp(std::format(R"(Core.Client {{"height":{},"width":{}}})", height, width));
}

void Session::setDebugLogging(const bool enabled) noexcept {
    debugLogging_ = enabled;
}

void Session::onTcpRead(std::span<const std::byte> data) {
    const genesis::network::TelnetFeedResult result = telnetCodec_.feed(data);
    writeWireReplies(result);

    if (result.negotiatedNow) {
        eventBus_.enqueueInboundEvent(Event{EventType::GmcpNegotiated, std::monostate{}});
    }
    enqueueInboundFromTelnet(result);
}

void Session::writeWireReplies(const genesis::network::TelnetFeedResult& telnetFeedResult) {
    if (!telnetFeedResult.wireReplies.empty()) {
        tcpSession_.send(telnetFeedResult.wireReplies);
    }
}

void Session::enqueueInboundFromTelnet(const genesis::network::TelnetFeedResult& telnetFeedResult) {
    for (const genesis::network::MudTextChunk& chunk : telnetFeedResult.textChunks) {
        eventBus_.enqueueInboundEvent(Event{EventType::MudText, chunk.text});
    }

    for (const genesis::network::GmcpPayload& payload : telnetFeedResult.gmcpPayloads) {
        eventBus_.enqueueInboundEvent(Event{EventType::GmcpRaw, payload.body});
    }
}

void Session::sendLine(std::string_view line) {
    tcpSession_.send(telnetCodec_.encodeLine(line));
}

} // namespace genesis::mudcore
