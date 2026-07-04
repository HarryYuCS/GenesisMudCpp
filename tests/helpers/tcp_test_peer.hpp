#pragma once

#include <boost/asio.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <span>
#include <vector>

namespace genesis::test {

enum class TcpTestPeerMode {
    Echo,
    PushOnConnect,
    CloseOnConnect,
    CloseAfterBytes,
    RecordWrites,
};

struct TcpTestPeerConfig {
    TcpTestPeerMode mode{TcpTestPeerMode::Echo};
    std::vector<std::byte> pushPayload;
    std::size_t closeAfterBytes{0};
};

/**
 * @brief Scripted loopback TCP peer for TcpSession black-box tests.
 *
 * Accepts connections repeatedly so reconnect scenarios can reuse one peer instance.
 */
class TcpTestPeer {
public:
    explicit TcpTestPeer(boost::asio::io_context& ioContext, TcpTestPeerConfig config = {})
        : ioContext_(ioContext)
        , config_(std::move(config))
        , acceptor_(ioContext, boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address("127.0.0.1"), 0)) {
        startAccept();
    }

    std::uint16_t port() const {
        return acceptor_.local_endpoint().port();
    }

    std::vector<std::byte> recordedWrites() const {
        const std::lock_guard<std::mutex> lock(recordMutex_);
        return recordedWrites_;
    }

    void resetRecordedWrites() {
        const std::lock_guard<std::mutex> lock(recordMutex_);
        recordedWrites_.clear();
    }

private:
    void startAccept() {
        auto socket = std::make_shared<boost::asio::ip::tcp::socket>(ioContext_);
        acceptor_.async_accept(*socket, [this, socket](const boost::system::error_code& error) {
            if (error) {
                return;
            }
            onClientConnected(std::move(socket));
        });
    }

    void onClientConnected(std::shared_ptr<boost::asio::ip::tcp::socket> socket) {
        switch (config_.mode) {
        case TcpTestPeerMode::CloseOnConnect:
            closeSocket(socket);
            startAccept();
            return;
        case TcpTestPeerMode::PushOnConnect:
            if (!config_.pushPayload.empty()) {
                boost::asio::async_write(
                    *socket,
                    boost::asio::buffer(config_.pushPayload.data(), config_.pushPayload.size()),
                    [this, socket](const boost::system::error_code& writeError, std::size_t) {
                        if (writeError) {
                            closeSocket(socket);
                            startAccept();
                            return;
                        }
                        startRead(socket);
                    });
            } else {
                startRead(socket);
            }
            return;
        case TcpTestPeerMode::Echo:
        case TcpTestPeerMode::RecordWrites:
        case TcpTestPeerMode::CloseAfterBytes:
            startRead(socket);
            return;
        }
    }

    void startRead(const std::shared_ptr<boost::asio::ip::tcp::socket>& socket) {
        auto buffer = std::make_shared<std::array<char, 1024>>();
        socket->async_read_some(
            boost::asio::buffer(*buffer),
            [this, socket, buffer](const boost::system::error_code& error, const std::size_t bytesRead) {
                if (error || bytesRead == 0) {
                    closeSocket(socket);
                    startAccept();
                    return;
                }

                if (config_.mode == TcpTestPeerMode::RecordWrites) {
                    appendRecorded(buffer->data(), bytesRead);
                }

                bytesReceived_ += bytesRead;
                const bool shouldCloseAfterEcho =
                    config_.mode == TcpTestPeerMode::CloseAfterBytes && bytesReceived_ >= config_.closeAfterBytes;

                boost::asio::async_write(
                    *socket,
                    boost::asio::buffer(buffer->data(), bytesRead),
                    [this, socket, shouldCloseAfterEcho](const boost::system::error_code& writeError, std::size_t) {
                        if (writeError || shouldCloseAfterEcho) {
                            closeSocket(socket);
                            startAccept();
                            return;
                        }
                        startRead(socket);
                    });
            });
    }

    void appendRecorded(const char* data, std::size_t size) {
        const std::lock_guard<std::mutex> lock(recordMutex_);
        for (std::size_t index = 0; index < size; ++index) {
            recordedWrites_.push_back(static_cast<std::byte>(static_cast<unsigned char>(data[index])));
        }
    }

    void closeSocket(const std::shared_ptr<boost::asio::ip::tcp::socket>& socket) {
        boost::system::error_code error;
        socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both, error);
        socket->close(error);
        bytesReceived_ = 0;
    }

    boost::asio::io_context& ioContext_;
    TcpTestPeerConfig config_;
    boost::asio::ip::tcp::acceptor acceptor_;

    mutable std::mutex recordMutex_;
    std::vector<std::byte> recordedWrites_;
    std::size_t bytesReceived_{0};
};

inline void appendSpan(std::vector<std::byte>& destination, std::span<const std::byte> data) {
    destination.insert(destination.end(), data.begin(), data.end());
}

} // namespace genesis::test
