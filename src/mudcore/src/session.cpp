#include <mudcore/session.hpp>
#include <mudcore/display_line.hpp>
#include <mudcore/event_bus.hpp>
#include <mudcore/inbound_pipeline.hpp>
#include <mudcore/outbound_pipeline.hpp>
#include <mudcore/game_state.hpp>
#include <mudcore/connection_life_cycle.hpp>

#include <network/telnet_codec.hpp>

#include <boost/asio/post.hpp>

#include <span>
#include <string_view>

#include <utility>

namespace genesis::mudcore {

Session::Session(boost::asio::io_context& ioContext)
    : ioContext_(ioContext)
    , tcpSession_(ioContext)
    , connectionLifeCycle_([this](std::string_view body) { sendGmcp(body); })
{
    // initialize the TCP session with own handlers
    tcpSession_.setReadHandler([this](std::span<const std::byte> data) { onTcpRead(data); });
    tcpSession_.setConnectedHandler([this]() {
        eventBus_.enqueueInboundEvent(Event{EventType::Connected, std::monostate{}});
    });
    tcpSession_.setDisconnectedHandler([this]() {
        // The codec is owned by the io thread; reset it here, not in poll().
        telnetCodec_.reset();
        eventBus_.enqueueInboundEvent(Event{EventType::Disconnected, std::monostate{}});
    });
    tcpSession_.setErrorHandler([this](const genesis::network::NetworkError& error) {
        eventBus_.enqueueInboundEvent(Event{EventType::Error, error});
    });
}

void Session::connect(const std::string& host, std::uint16_t port) {
    // post the connection request (thread safe) on the io context
    connectionLifeCycle_.onConnectRequested();
    boost::asio::post(ioContext_, [this, host, port]() {
        tcpSession_.connect(genesis::network::ConnectionConfig{host, port});
    });
}

void Session::disconnect() {
    // post the disconnect request (thread safe) on the io context
    connectionLifeCycle_.onTcpDisconnected();
    boost::asio::post(ioContext_, [this]() { tcpSession_.disconnect(); });
}

PollResult Session::poll() {
    // create empty result object
    PollResult result;

    // loop through the vector of eventBus events and process them
    for (Event& event : eventBus_.drainInboundEvents()) {
        switch (event.type) {
        case EventType::Connected:
            connectionLifeCycle_.onTcpConnected();
            result.lines.push_back(DisplayLine{OutputSink::System, "Connected."});
            break;
        case EventType::Disconnected:
            connectionLifeCycle_.onTcpDisconnected();
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
            // if Mudtext, delegate to inbound pipeline to match triggers and generate displayline
            if (const auto* text = std::get_if<std::string>(&event.payload)) {
                if (auto line = inboundPipeline_.processMudText(*text)) {
                    result.lines.push_back(std::move(*line));
                }
            }
            break;
        }
        case EventType::GmcpRaw: {
            // if GmcpRaw, delegate to inbound pipeline to handle Gmcp and mutate game state if necessary
            if (const auto* raw = std::get_if<std::string>(&event.payload)) {
                // GmcpResult contains optional displayline and boolean indicating state change (to nudge for a refresh)
                auto gmcpResult = inboundPipeline_.processGmcp(*raw, gameState_);
                if (gmcpResult.line.has_value()) {
                    result.lines.push_back(std::move(*gmcpResult.line));
                }
                if (gmcpResult.stateChanged) {
                    result.stateChanged = true;
                }
                // React only to the effects the pipeline reports; onPlayerLoggedIn is
                // idempotent and internally guards the HandshakeSent -> Ready transition.
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
        // thread safe post, moves owrnership of line to the io context, completion handler
        // is own sendLine member
        boost::asio::post(ioContext_, [this, line = std::move(*line)]() { sendLine(line); });
    }
}

void Session::sendGmcp(const std::string_view body) {
    boost::asio::post(ioContext_, [this, payload = std::string(body)]() {
        tcpSession_.send(telnetCodec_.encodeGmcp(payload));
    });
}

void Session::onTcpRead(std::span<const std::byte> data) {
    const genesis::network::TelnetFeedResult result = telnetCodec_.feed(data);
    writeWireReplies(result);

    // GMCP negotiation (IAC WILL GMCP) necessarily precedes any GMCP subnegotiation
    // frame in the byte stream, so signal it before the payloads parsed in this feed.
    // This keeps the lifecycle ordered (GmcpEnabled -> HandshakeSent) even when a server
    // pipelines negotiation and a GMCP frame in a single TCP segment.
    if (result.negotiatedNow) {
        eventBus_.enqueueInboundEvent(Event{EventType::GmcpNegotiated, std::monostate{}});
    }
    enqueueInboundFromTelnet(result);
}

void Session::writeWireReplies(const genesis::network::TelnetFeedResult& telnetFeedResult) {
    // legal to directly send since this is run from inside io thread!
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
