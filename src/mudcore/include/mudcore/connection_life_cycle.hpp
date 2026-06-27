#ifndef MUDCORE_CONNECTION_LIFE_CYCLE_HPP
#define MUDCORE_CONNECTION_LIFE_CYCLE_HPP

#include <functional>
#include <string_view>

namespace genesis::mudcore {

enum class ConnectionPhase {
    Disconnected,
    Connecting,
    Connected,
    GmcpEnabled,
    HandshakeSent,
    Ready,
};

/**
 * @brief Genesis connection protocol policy (handshake sequencing and phase tracking).
 *
 * Session owns I/O; this class owns *when* to send Core.Hello / Core.Supports.Set.
 */
class ConnectionLifeCycle {
public:
    using SendGmcpFunc = std::function<void(std::string_view body)>;

    explicit ConnectionLifeCycle(SendGmcpFunc sendGmcp);

    ConnectionPhase phase() const noexcept;

    void onConnectRequested();
    void onTcpConnected();
    void onGmcpNegotiated();
    void onTcpDisconnected();

private:
    void sendGenesisHandshake();

    ConnectionPhase phase_{ConnectionPhase::Disconnected};
    SendGmcpFunc sendGmcp_;
};

} // namespace genesis::mudcore

#endif // MUDCORE_CONNECTION_LIFE_CYCLE_HPP
