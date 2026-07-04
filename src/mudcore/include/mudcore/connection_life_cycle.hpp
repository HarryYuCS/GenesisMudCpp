/**
 * @file connection_life_cycle.hpp
 * @brief Genesis-specific connection protocol phases and GMCP handshake sequencing.
 */

#ifndef MUDCORE_CONNECTION_LIFE_CYCLE_HPP
#define MUDCORE_CONNECTION_LIFE_CYCLE_HPP

#include <functional>
#include <string_view>

namespace genesis::mudcore {

/**
 * @brief Application-level connection milestone (distinct from network::ConnectionState).
 */
enum class ConnectionPhase {
    Disconnected,  ///< No active session.
    Connecting,    ///< connect() called; awaiting TCP.
    Connected,     ///< TCP up; awaiting telnet GMCP negotiation.
    GmcpEnabled,   ///< IAC DO GMCP sent; about to send Core handshake.
    HandshakeSent, ///< Core.Hello and Core.Supports.Set have been sent.
    Ready,         ///< Normal gameplay (optional; may follow HandshakeSent).
};

/**
 * @brief Tracks connection phase and triggers the Genesis GMCP handshake.
 *
 * Session provides I/O via SendGmcpFunc. This class decides *when* to send
 * Core.Hello and Core.Supports.Set. All edge handlers are idempotent.
 */
class ConnectionLifeCycle {
public:
    /** @brief Callback invoked to send a GMCP message body (Session encodes and writes). */
    using SendGmcpFunc = std::function<void(std::string_view body)>;

    /**
     * @brief Construct with a callback for outbound GMCP bodies.
     * @param sendGmcp Called on the io thread to transmit a GMCP subnegotiation payload.
     */
    explicit ConnectionLifeCycle(SendGmcpFunc sendGmcp);

    /**
     * @brief Current application connection phase.
     * @return The active ConnectionPhase.
     */
    ConnectionPhase phase() const noexcept;

    /**
     * @brief User requested a connection (Session::connect on main thread).
     *
     * Transitions Disconnected -> Connecting. No-op if not Disconnected.
     */
    void onConnectRequested();

    /**
     * @brief TCP connection succeeded.
     *
     * Transitions Connecting -> Connected. No-op if not Connecting.
     */
    void onTcpConnected();

    /**
     * @brief Telnet GMCP option was negotiated (IAC WILL GMCP / IAC DO GMCP).
     *
     * Sends Core.Hello and Core.Supports.Set via sendGmcp. No-op if not Connected.
     */
    void onGmcpNegotiated();

    /**
     * @brief TCP connection ended or disconnect() was called.
     *
     * Resets phase to Disconnected.
     */
    void onTcpDisconnected();

    /**
     * @brief Player logged in (Char.Login broadcast received).
     *
     * Transitions HandshakeSent -> Ready. No-op if not HandshakeSent.
     */
    void onPlayerLoggedIn();

private:
    /** @brief Send Core.Hello and Core.Supports.Set; sets phase to HandshakeSent. */
    void sendGenesisHandshake();

    ConnectionPhase phase_{ConnectionPhase::Disconnected};
    SendGmcpFunc sendGmcp_;
};

} // namespace genesis::mudcore

#endif // MUDCORE_CONNECTION_LIFE_CYCLE_HPP
