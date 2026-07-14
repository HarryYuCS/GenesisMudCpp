/**
 * @file session.hpp
 * @brief Central orchestrator between network I/O, mudcore logic, and the GUI.
 *
 * Thread model:
 * - Main thread: connect(), disconnect(), sendCommand(), sendGmcp(), poll(), gameState().
 *   ConnectionLifeCycle and GameState are mutated only here (during poll() or connect()/disconnect()).
 * - Io thread: all TcpSession and TelnetCodec access. onTcpRead() runs via the TcpSession read
 *   handler; connect/disconnect/sends are posted from the main thread with asio::post.
 * - Bridge: io-thread observations (Connected, Disconnected, Error, GmcpNegotiated, MudText,
 *   GmcpRaw) travel through EventBus and are consumed by poll().
 */

#ifndef MUDCORE_SESSION_HPP
#define MUDCORE_SESSION_HPP

#include <mudcore/connection_life_cycle.hpp>
#include <mudcore/display_line.hpp>
#include <mudcore/event_bus.hpp>
#include <mudcore/game_state.hpp>
#include <mudcore/inbound_pipeline.hpp>
#include <mudcore/gmcp_parser.hpp>
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

/**
 * @brief Result of one poll() call for the GUI to consume.
 */
struct PollResult {
    std::vector<DisplayLine> lines;                              ///< New text to render this frame.
    bool stateChanged{false};                                    ///< true if GameState was updated; refresh map/status.
    ConnectionPhase connectionPhase{ConnectionPhase::Disconnected}; ///< For connection indicator in the UI.
};

/**
 * @brief Manages the MUD session: TCP, telnet, GMCP handshake, event bridging, and pipelines.
 *
 * This is the only mudcore type the GUI should interact with directly.
 */
class Session {
public:
    /**
     * @brief Construct a session bound to an io_context for async network I/O.
     * @param ioContext Shared io_context run on the network thread.
     */
    explicit Session(boost::asio::io_context& ioContext);

    Session(const Session&) = delete;
    Session(Session&&) = delete;
    Session& operator=(const Session&) = delete;
    Session& operator=(Session&&) = delete;
    ~Session() = default;

    /**
     * @brief Start an async connection to the MUD server.
     *
     * Main thread only. Updates ConnectionLifeCycle to Connecting.
     *
     * @param host Hostname (e.g. "mud.genesismud.org").
     * @param port Port number (e.g. 3011).
     */
    void connect(const std::string& host, std::uint16_t port);

    /**
     * @brief Close the TCP connection and reset connection phase.
     *
     * Main thread only.
     */
    void disconnect();

    /**
     * @brief Drain inbound events and produce display output for the GUI.
     *
     * Call from a wxTimer on the main thread (~30 ms). Runs pipelines and updates GameState.
     *
     * @return Lines to render and flags for state/connection UI updates.
     */
    PollResult poll();

    /**
     * @brief Read-only access to structured game state for map and status panels.
     * @return Current GameState snapshot.
     */
    const GameState& gameState() const noexcept;

    /**
     * @brief Current application connection phase.
     * @return Phase from ConnectionLifeCycle.
     */
    ConnectionPhase connectionPhase() const noexcept;

    /**
     * @brief Send a user command to the MUD.
     *
     * Runs OutboundPipeline on the main thread, then posts the encoded line to the io thread.
     *
     * @param command Raw input from the input bar.
     */
    void sendCommand(std::string_view command);

    /**
     * @brief Send a GMCP message body to the server.
     *
     * Main thread only. Posted to the io thread for telnet encoding and transmission.
     *
     * @param body GMCP payload (e.g. R"(Char.Login {"name":"x","password":"y"})").
     */
    void sendGmcp(std::string_view body);

    /**
     * @brief Inform the server of the client text window size in character cells.
     *
     * Sends Core.Client. Main thread only.
     *
     * @param width  Character columns.
     * @param height Character rows.
     */
    void sendClientSize(unsigned width, unsigned height);

    /** @brief Enable GMCP package logging in poll() for diagnostics. Main thread only. */
    void setDebugLogging(bool enabled) noexcept;

private:
    /**
     * @brief TcpSession read handler (io thread).
     *
     * Feeds TelnetCodec, writes wire replies, and enqueues inbound events
     * (including GmcpNegotiated, which poll() turns into the Core handshake).
     *
     * @param data Raw bytes received from the socket.
     */
    void onTcpRead(std::span<const std::byte> data);

    /**
     * @brief Send telnet protocol auto-replies immediately (io thread).
     * @param telnetFeedResult Result from TelnetCodec::feed containing wireReplies.
     */
    void writeWireReplies(const genesis::network::TelnetFeedResult& telnetFeedResult);

    /**
     * @brief Convert telnet output into inbound Events and enqueue them (io thread).
     * @param telnetFeedResult Parsed text chunks and GMCP payloads from TelnetCodec.
     */
    void enqueueInboundFromTelnet(const genesis::network::TelnetFeedResult& telnetFeedResult);

    /**
     * @brief Encode and send a user command line with telnet CRLF framing (io thread).
     * @param line Command text without trailing newline.
     */
    void sendLine(std::string_view line);

    boost::asio::io_context& ioContext_;

    // telnetCodec_ and eventBus_ must outlive tcpSession_: its destructor can fire
    // the disconnected handler, which touches both.
    genesis::network::TelnetCodec telnetCodec_;
    EventBus eventBus_;
    genesis::network::TcpSession tcpSession_;

    InboundPipeline inboundPipeline_;
    OutboundPipeline outboundPipeline_;
    ConnectionLifeCycle connectionLifeCycle_;

    GameState gameState_;
    GmcpParser gmcpParser_;
    bool debugLogging_{false};
};

} // namespace genesis::mudcore

#endif // MUDCORE_SESSION_HPP
