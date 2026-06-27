#ifndef MUDCORE_SESSION_HPP
#define MUDCORE_SESSION_HPP

#include <mudcore/connection_life_cycle.hpp>
#include <mudcore/display_line.hpp>
#include <mudcore/event_bus.hpp>
#include <mudcore/game_state.hpp>
#include <mudcore/gmcp_parser.hpp>
#include <mudcore/inbound_pipeline.hpp>
#include <mudcore/outbound_pipeline.hpp>
#include <network/network_types.hpp>
#include <network/tcp_session.hpp>
#include <network/telnet_codec.hpp>

#include <boost/asio/io_context.hpp>

#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace genesis::mudcore {

struct PollResult {
    std::vector<DisplayLine> lines;
    bool stateChanged{false};
    ConnectionPhase connectionPhase{ConnectionPhase::Disconnected};
};

/**
 * @brief Orchestrates network I/O, thread bridging, pipelines, and connection lifecycle.
 *
 * Main thread: connect, disconnect, sendCommand, poll, gameState.
 * Io thread: onTcpRead (via TcpSession read handler).
 */
class Session {
public:
    explicit Session(boost::asio::io_context& ioContext);
    Session(const Session&) = delete;
    Session(Session&&) = delete;
    Session& operator=(const Session&) = delete;
    Session& operator=(Session&&) = delete;
    ~Session() = default;

    void connect(const std::string& host, std::uint16_t port);
    void disconnect();

    PollResult poll();
    const GameState& gameState() const noexcept;
    ConnectionPhase connectionPhase() const noexcept;

    void sendCommand(std::string_view command);

private:
    void onTcpRead(std::span<const std::byte> data);
    void writeWireReplies(const genesis::network::TelnetFeedResult& telnetFeedResult);
    void enqueueInboundFromTelnet(const genesis::network::TelnetFeedResult& telnetFeedResult);

    void sendGmcp(std::string_view body);
    void sendLine(std::string_view line);

    boost::asio::io_context& ioContext_;
    genesis::network::TelnetCodec telnetCodec_;
    genesis::network::TcpSession tcpSession_;

    InboundPipeline inboundPipeline_;
    OutboundPipeline outboundPipeline_;
    GmcpParser gmcpParser_;
    ConnectionLifeCycle connectionLifeCycle_;

    EventBus eventBus_;
    GameState gameState_;
};

} // namespace genesis::mudcore

#endif // MUDCORE_SESSION_HPP
