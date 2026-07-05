/**
 * @file tcp_session.hpp
 * @brief Async TCP socket wrapper using Boost.Asio.
 *
 * Threading contract: connect(), disconnect(), and send() must run on the io_context
 * thread (callers post from other threads). getConnectionState() may be called from
 * any thread. All handlers are invoked on the io_context thread.
 */

#ifndef NETWORK_TCP_SESSION_HPP
#define NETWORK_TCP_SESSION_HPP

#include <boost/asio.hpp>
#include <network/network_types.hpp>

#include <array>
#include <atomic>
#include <cstddef>
#include <deque>
#include <functional>
#include <span>
#include <vector>

namespace genesis::network {

/**
 * @brief Manages async TCP connect, read loop, and a serialized write queue.
 *
 * The internal Asio completion handler invokes the user-supplied ReadHandler with payload bytes.
 * ReadHandler must not be the raw Asio callback — TcpSession wraps error handling and read chaining.
 */
class TcpSession {
public:
    /** @brief Application callback invoked with received payload bytes (io thread). */
    using ReadHandler = std::function<void(std::span<const std::byte> data)>;
    using ConnectedHandler = std::function<void()>;
    using DisconnectedHandler = std::function<void()>;
    using ErrorHandler = std::function<void(NetworkError)>;

    /**
     * @brief Construct a session bound to an io_context.
     * @param ioContext io_context that will run async operations for this socket.
     */
    explicit TcpSession(boost::asio::io_context& ioContext);

    ~TcpSession();

    /**
     * @brief Start async resolve and connect. No-op unless currently DISCONNECTED.
     * @param config Target host and port.
     */
    void connect(const ConnectionConfig& config);

    /**
     * @brief Close the socket and stop the read loop. Pending writes are dropped.
     */
    void disconnect();

    /**
     * @brief Queue raw bytes for async transmission.
     *
     * Writes are serialized: if a write is in flight the data is queued and sent
     * in order. Write failures are reported via the ErrorHandler and close the
     * connection. No-op when not connected or when @p data is empty.
     *
     * @param data Bytes to write (telnet-framed data from TelnetCodec).
     */
    void send(std::span<const std::byte> data);

    /**
     * @brief Register the handler called for each successful read.
     *
     * Typically set once by Session to wire onTcpRead(). The read loop only runs
     * while a read handler is registered; without one, peer disconnects are not
     * detected until a write fails.
     *
     * @param readHandler Callback invoked on the io thread with received bytes.
     */
    void setReadHandler(ReadHandler readHandler);

    /** @brief Invoked on the io thread after TCP connect succeeds. */
    void setConnectedHandler(ConnectedHandler handler);

    /** @brief Invoked on the io thread exactly once per connection when the socket closes. */
    void setDisconnectedHandler(DisconnectedHandler handler);

    /** @brief Invoked on the io thread when an async operation fails (always before disconnect). */
    void setErrorHandler(ErrorHandler handler);

    /**
     * @brief Current TCP connection state (safe from any thread).
     * @return ConnectionState snapshot.
     */
    ConnectionState getConnectionState() const;

private:
    /** @brief Start the read operation. */
    void startRead();

    /** @brief Start the next write operation from the pending writes queue. */
    void startNextWrite();

    void onResolveComplete(
        const boost::system::error_code& error,
        const boost::asio::ip::tcp::resolver::results_type& results);

    void onConnectComplete(
        const boost::system::error_code& error,
        const boost::asio::ip::tcp::endpoint& endpoint);

    void onReadComplete(const boost::system::error_code& error, std::size_t bytesRead);

    void onWriteComplete(const boost::system::error_code& error, std::size_t bytesTransferred);

    static bool isOperationAborted(const boost::system::error_code& error);

    /** @brief Report a fatal async failure, close the socket, and notify disconnect. */
    void failConnection(const boost::system::error_code& error);

    /** @brief Close the socket after a clean peer EOF and notify disconnect. */
    void onPeerDisconnected();

    void closeSocketAndNotify();
    void notifyDisconnected();

    std::atomic<ConnectionState> connectionState_;
    boost::asio::io_context& ioContext_;

    // socket for the TCP connection
    boost::asio::ip::tcp::socket socket_;

    // resolver for DNS
    boost::asio::ip::tcp::resolver resolver_;

    // buffer for read operations
    std::array<std::byte, 1024> readBuffer_;

    // handlers
    ReadHandler readHandler_;
    ConnectedHandler connectedHandler_;
    DisconnectedHandler disconnectedHandler_;
    ErrorHandler errorHandler_;

    // queue of pending writes to be sent
    std::deque<std::vector<std::byte>> pendingWrites_;

    // track pending writes and read to prevent concurrent read/write race conditions
    bool writeInProgress_{false};
    bool readInProgress_{false};
    bool disconnectNotified_{false};
};

} // namespace genesis::network

#endif // NETWORK_TCP_SESSION_HPP
