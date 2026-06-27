#include <mudcore/session.hpp>

#include <utility>

namespace genesis::mudcore {

Session::Session(boost::asio::io_context& ioContext)
    : ioContext_(ioContext)
    , tcpSession_(ioContext)
    , connectionLifeCycle_([this](std::string_view body) { sendGmcp(body); })
{
    tcpSession_.setReadHandler([this](std::span<const std::byte> data) { onTcpRead(data); });
}

void Session::connect(const std::string& host, std::uint16_t port) {
    connectionLifeCycle_.onConnectRequested();
    tcpSession_.connect(genesis::network::ConnectionConfig{host, port});
}

void Session::disconnect() {
    tcpSession_.disconnect();
    connectionLifeCycle_.onTcpDisconnected();
}

PollResult Session::poll() {
    PollResult result;
    result.connectionPhase = connectionLifeCycle_.phase();

    for (Event& event : eventBus_.drainInboundEvents()) {
        switch (event.type) {
        case EventType::Connected:
            break;
        case EventType::Disconnected:
            connectionLifeCycle_.onTcpDisconnected();
            break;
        case EventType::Error:
            break;
        case EventType::MudText: {
            if (const auto* text = std::get_if<std::string>(&event.payload)) {
                auto lines = inboundPipeline_.processMudText(*text);
                result.lines.insert(result.lines.end(), lines.begin(), lines.end());
            }
            break;
        }
        case EventType::GmcpRaw: {
            if (const auto* raw = std::get_if<std::string>(&event.payload)) {
                if (auto message = gmcpParser_.parse(*raw)) {
                    const auto before = gameState_.room().roomId;
                    auto lines = inboundPipeline_.processGmcp(*message, gameState_);
                    result.lines.insert(result.lines.end(), lines.begin(), lines.end());
                    if (gameState_.room().roomId != before) {
                        result.stateChanged = true;
                    }
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

void Session::onTcpRead(std::span<const std::byte> data) {
    const genesis::network::TelnetFeedResult result = telnetCodec_.feed(data);
    writeWireReplies(result);
    enqueueInboundFromTelnet(result);

    if (result.negotiatedNow) {
        connectionLifeCycle_.onGmcpNegotiated();
    }
}

void Session::writeWireReplies(const genesis::network::TelnetFeedResult& telnetFeedResult) {
    if (!telnetFeedResult.wireReplies.empty()) {
        tcpSession_.send(telnetFeedResult.wireReplies);
    }
}

void Session::enqueueInboundFromTelnet(const genesis::network::TelnetFeedResult& telnetFeedResult) {
    for (const genesis::network::MudTextChunk& chunk : telnetFeedResult.textChunks) {
        Event event;
        event.type = EventType::MudText;
        event.payload = chunk.text;
        eventBus_.enqueueInboundEvent(std::move(event));
    }

    for (const genesis::network::GmcpPayload& payload : telnetFeedResult.gmcpPayloads) {
        Event event;
        event.type = EventType::GmcpRaw;
        event.payload = payload.body;
        eventBus_.enqueueInboundEvent(std::move(event));
    }
}

void Session::sendGmcp(std::string_view body) {
    tcpSession_.send(telnetCodec_.encodeGmcp(body));
}

void Session::sendLine(std::string_view line) {
    tcpSession_.send(telnetCodec_.encodeLine(line));
}

} // namespace genesis::mudcore
