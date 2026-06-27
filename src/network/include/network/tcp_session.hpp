#ifndef NETWORK_TCP_SESSION_HPP
#define NETWORK_TCP_SESSION_HPP

#include <boost/asio.hpp>
#include <network/network_types.hpp>
#include <string>
#include <functional>
#include <span>
#include <cstddef>

namespace genesis::network {

/**
 * @brief The TcpSession class is responsible for managing the TCP session with the server.
 *
 * Wraps the async logic for writing to and reading from a TCP socket.
 */
class TcpSession {
public:
    using ReadHandler = std::function<void(std::span<const std::byte> data)>; // callback to be registered by user, separate from boost handler

    /**
     * @brief Constructs a new TcpSession object.
     *
     * @param ioContext The IO context to use for the session.
     * @param host The host to connect to.
     * @param port The port to connect to.
     * @param readHandler The callback to use for reading data.
     */
    explicit TcpSession(boost::asio::io_context& ioContext);
    ~TcpSession();

    void connect(const ConnectionConfig& config);
    void disconnect();

    void send(std::span<const std::byte> data);

    void setReadHandler(ReadHandler readHandler);

    ConnectionState getConnectionState() const;

private:
    ConnectionState connectionState_;
    boost::asio::io_context& ioContext_;
    boost::asio::ip::tcp::socket socket_;
    boost::asio::ip::tcp::resolver resolver_;

    std::array<std::byte, 1024> readBuffer_;
    // internal lambda calls read handler
    ReadHandler readHandler_;

    void onReadError(const boost::system::error_code& error);
    void onPeerDisconnected();
};

} // namespace genesis::network

#endif // NETWORK_TCP_SESSION_HPP