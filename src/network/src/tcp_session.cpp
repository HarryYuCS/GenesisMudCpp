#include <network/tcp_session.hpp>

#include <boost/system/error_code.hpp>

#include <string>
#include <utility>

namespace genesis::network {

TcpSession::TcpSession(boost::asio::io_context& ioContext)
    : connectionState_(ConnectionState::DISCONNECTED)
    , ioContext_(ioContext)
    , socket_(ioContext)
    , resolver_(ioContext) {}

TcpSession::~TcpSession() {
    disconnect();
}

void TcpSession::connect(const ConnectionConfig& config) {
    if (connectionState_ != ConnectionState::DISCONNECTED) {
        return;
    }

    disconnectNotified_ = false;
    connectionState_ = ConnectionState::RESOLVING;

    resolver_.async_resolve(
        config.host,
        std::to_string(config.port),
        [this](const boost::system::error_code& error, const boost::asio::ip::tcp::resolver::results_type& results) {
            if (error) {
                failConnection(error);
                return;
            }

            connectionState_ = ConnectionState::CONNECTING;
            boost::asio::async_connect(
                socket_,
                results,
                [this](const boost::system::error_code& connectError, const boost::asio::ip::tcp::endpoint&) {
                    if (connectError) {
                        failConnection(connectError);
                        return;
                    }

                    connectionState_ = ConnectionState::CONNECTED;
                    if (connectedHandler_) {
                        connectedHandler_();
                    }
                    startRead();
                });
        });
}

void TcpSession::disconnect() {
    if (connectionState_ == ConnectionState::DISCONNECTED) {
        return;
    }

    boost::system::error_code error;
    socket_.cancel(error);
    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, error);
    closeSocketAndNotify();
}

void TcpSession::send(const std::span<const std::byte> data) {
    if (connectionState_ != ConnectionState::CONNECTED || data.empty()) {
        return;
    }

    pendingWrites_.emplace_back(data.begin(), data.end());
    startNextWrite();
}

void TcpSession::setReadHandler(ReadHandler readHandler) {
    readHandler_ = std::move(readHandler);
}

void TcpSession::setConnectedHandler(ConnectedHandler handler) {
    connectedHandler_ = std::move(handler);
}

void TcpSession::setDisconnectedHandler(DisconnectedHandler handler) {
    disconnectedHandler_ = std::move(handler);
}

void TcpSession::setErrorHandler(ErrorHandler handler) {
    errorHandler_ = std::move(handler);
}

ConnectionState TcpSession::getConnectionState() const {
    return connectionState_.load();
}

void TcpSession::startRead() {
    if (connectionState_ != ConnectionState::CONNECTED || readInProgress_ || !readHandler_) {
        return;
    }

    readInProgress_ = true;
    socket_.async_read_some(
        boost::asio::buffer(readBuffer_),
        [this](const boost::system::error_code& error, const std::size_t bytesRead) {
            readInProgress_ = false;

            if (error) {
                if (error == boost::asio::error::operation_aborted) {
                    return;
                }
                if (error == boost::asio::error::eof) {
                    onPeerDisconnected();
                    return;
                }
                failConnection(error);
                return;
            }

            if (bytesRead == 0) {
                onPeerDisconnected();
                return;
            }

            readHandler_(std::span<const std::byte>(readBuffer_.data(), bytesRead));
            startRead();
        });
}

void TcpSession::startNextWrite() {
    if (writeInProgress_ || pendingWrites_.empty()) {
        return;
    }

    writeInProgress_ = true;
    const std::vector<std::byte>& buffer = pendingWrites_.front();
    boost::asio::async_write(
        socket_,
        boost::asio::buffer(buffer.data(), buffer.size()),
        [this](const boost::system::error_code& error, std::size_t) {
            writeInProgress_ = false;

            if (error) {
                if (error == boost::asio::error::operation_aborted) {
                    return;
                }
                failConnection(error);
                return;
            }

            pendingWrites_.pop_front();
            startNextWrite();
        });
}

void TcpSession::failConnection(const boost::system::error_code& error) {
    if (error == boost::asio::error::operation_aborted) {
        return;
    }

    if (errorHandler_) {
        errorHandler_(NetworkError{error.message()});
    }
    closeSocketAndNotify();
}

void TcpSession::onPeerDisconnected() {
    closeSocketAndNotify();
}

void TcpSession::closeSocketAndNotify() {
    boost::system::error_code closeError;
    socket_.close(closeError);
    connectionState_ = ConnectionState::DISCONNECTED;
    readInProgress_ = false;
    writeInProgress_ = false;
    pendingWrites_.clear();
    notifyDisconnected();
}

void TcpSession::notifyDisconnected() {
    if (disconnectNotified_) {
        return;
    }
    disconnectNotified_ = true;
    if (disconnectedHandler_) {
        disconnectedHandler_();
    }
}

} // namespace genesis::network
