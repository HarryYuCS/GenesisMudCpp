#include <network/tcp_session.hpp>

namespace genesis::network {

TcpSession::TcpSession(boost::asio::io_context& ioContext)
    : connectionState_(ConnectionState::DISCONNECTED)
    , ioContext_(ioContext)
    , socket_(ioContext)
    , resolver_(ioContext) {}

TcpSession::~TcpSession() = default;

void TcpSession::connect(const ConnectionConfig& config) {
    (void)config;
    connectionState_ = ConnectionState::CONNECTING;
}

void TcpSession::disconnect() {
    connectionState_ = ConnectionState::DISCONNECTED;
}

void TcpSession::send(std::span<const std::byte> data) {
    (void)data;
}

void TcpSession::setReadHandler(ReadHandler readHandler) {
    readHandler_ = std::move(readHandler);
}

ConnectionState TcpSession::getConnectionState() const {
    return connectionState_;
}

void TcpSession::onReadError(const boost::system::error_code& error) {
    (void)error;
}

void TcpSession::onPeerDisconnected() {}

} // namespace genesis::network
