/**
 * @file tcp_session.hpp
 * @brief Async TCP socket wrapper using Boost.Asio.
 *
 * All methods that touch the socket are intended to run on the io_context thread.
 */

#ifndef NETWORK_TCP_SESSION_HPP
#define NETWORK_TCP_SESSION_HPP

#include <boost/asio.hpp>
#include <network/network_types.hpp>

#include <array>
#include <cstddef>
#include <functional>
#include <span>
#include <string>

namespace genesis::network {

/**
 * @brief Manages async TCP connect, read loop, and writes.
 *
 * The internal Asio completion handler invokes the user-supplied ReadHandler with payload bytes.
 * ReadHandler must not be the raw Asio callback — TcpSession wraps error handling and read chaining.
 */
class TcpSession {
public:
    /** @brief Application callback invoked with received payload bytes (io thread). */
    using ReadHandler = std::function<void(std::span<const std::byte> data)>;

    /**
     * @brief Construct a session bound to an io_context.
     * @param ioContext io_context that will run async operations for this socket.
     */
    explicit TcpSession(boost::asio::io_context& ioContext);

    ~TcpSession();

    /**
     * @brief Start async resolve and connect.
     * @param config Target host and port.
     */
    void connect(const ConnectionConfig& config);

    /**
     * @brief Close the socket and stop the read loop.
     */
    void disconnect();

    /**
     * @brief Send raw bytes on the socket (async).
     * @param data Bytes to write (telnet-framed data from TelnetCodec).
     */
    void send(std::span<const std::byte> data);

    /**
     * @brief Register the handler called for each successful read.
     *
     * Typically set once by Session to wire onTcpRead().
     *
     * @param readHandler Callback invoked on the io thread with received bytes.
     */
    void setReadHandler(ReadHandler readHandler);

    /**
     * @brief Current TCP connection state.
     * @return ConnectionState snapshot.
     */
    ConnectionState getConnectionState() const;

private:
    ConnectionState connectionState_;
    boost::asio::io_context& ioContext_;
    boost::asio::ip::tcp::socket socket_;
    boost::asio::ip::tcp::resolver resolver_;

    std::array<std::byte, 1024> readBuffer_;
    ReadHandler readHandler_;

    /** @brief Handle a read error; stop chaining reads and notify disconnect. */
    void onReadError(const boost::system::error_code& error);

    /** @brief Handle graceful peer close (zero-byte read). */
    void onPeerDisconnected();
};

} // namespace genesis::network

#endif // NETWORK_TCP_SESSION_HPP
