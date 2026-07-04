#include <mudcore/connection_life_cycle.hpp>

namespace genesis::mudcore {

ConnectionLifeCycle::ConnectionLifeCycle(SendGmcpFunc sendGmcp)
    : sendGmcp_(std::move(sendGmcp)) {}

ConnectionPhase ConnectionLifeCycle::phase() const noexcept {
    return phase_;
}

void ConnectionLifeCycle::onConnectRequested() {
    if (phase_ != ConnectionPhase::Disconnected) {
        return;
    }
    phase_ = ConnectionPhase::Connecting;
}

void ConnectionLifeCycle::onTcpConnected() {
    if (phase_ != ConnectionPhase::Connecting) {
        return;
    }
    phase_ = ConnectionPhase::Connected;
}

void ConnectionLifeCycle::onGmcpNegotiated() {
    if (phase_ != ConnectionPhase::Connected) {
        return;
    }
    phase_ = ConnectionPhase::GmcpEnabled;
    sendGenesisHandshake();
}

void ConnectionLifeCycle::onTcpDisconnected() {
    phase_ = ConnectionPhase::Disconnected;
}

void ConnectionLifeCycle::onPlayerLoggedIn() {
    if (phase_ == ConnectionPhase::HandshakeSent) {
        phase_ = ConnectionPhase::Ready;
    }
}

void ConnectionLifeCycle::sendGenesisHandshake() {
    sendGmcp_(R"(Core.Hello {"client":"GenesisCpp","version":"0.1"})");
    sendGmcp_(R"(Core.Supports.Set ["Char 1","Room 1","Comm 1","Core 1"])");
    phase_ = ConnectionPhase::HandshakeSent;
}

} // namespace genesis::mudcore
