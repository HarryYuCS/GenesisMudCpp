#pragma once

#include <network/tcp_session.hpp>
#include <tcp_test_peer.hpp>

#include <boost/asio.hpp>

#include <atomic>
#include <chrono>
#include <functional>
#include <thread>

namespace genesis::test {

inline constexpr std::chrono::milliseconds kDefaultTestTimeout{500};

/** @brief Wait until a predicate is true or a timeout is reached. */
inline bool waitUntil(const std::function<bool()>& predicate, const std::chrono::milliseconds timeout) {
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    while (!predicate() && std::chrono::steady_clock::now() < deadline) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return predicate();
}

/** @brief Wait until a flag is true or a timeout is reached. */
inline bool waitUntil(const std::atomic<bool>& flag, const std::chrono::milliseconds timeout) {
    return waitUntil([&]() { return flag.load(); }, timeout);
}

/** @brief Convert a span of bytes to a string. */
inline std::string bytesToString(std::span<const std::byte> data) {
    std::string text;
    text.reserve(data.size());
    for (const std::byte byte : data) {
        text.push_back(static_cast<char>(byte));
    }
    return text;
}

/**
 * @brief RAII harness for TcpSession black-box tests on a shared io_context thread.
 */
class TcpSessionHarness {
public:
    explicit TcpSessionHarness(TcpTestPeerConfig peerConfig = {})
        : peer_(io_, std::move(peerConfig))
        , session_(io_) {}

    ~TcpSessionHarness() {
        stop();
    }

    TcpTestPeer& peer() noexcept {
        return peer_;
    }

    genesis::network::TcpSession& session() noexcept {
        return session_;
    }

    void start() {
        if (started_) {
            return;
        }
        started_ = true;
        ioThread_ = std::thread([this]() { io_.run(); });
    }

    void stop() {
        session_.disconnect();
        io_.stop();
        if (ioThread_.joinable()) {
            ioThread_.join();
        }
        started_ = false;
    }

    void connectToPeer() {
        session_.connect(genesis::network::ConnectionConfig{"127.0.0.1", peer_.port()});
    }

    bool waitForConnection(const std::chrono::milliseconds timeout = kDefaultTestTimeout) {
        return waitUntil(connected_, timeout);
    }

    bool waitForDisconnection(const std::chrono::milliseconds timeout = kDefaultTestTimeout) {
        return waitUntil(disconnected_, timeout);
    }

    void trackConnected() {
        session_.setConnectedHandler([this]() {
            ++connectedCount_;
            connected_.store(true);
        });
    }

    void trackDisconnected() {
        session_.setDisconnectedHandler([this]() {
            ++disconnectedCount_;
            disconnected_.store(true);
        });
    }

    void trackError() {
        session_.setErrorHandler([this](const genesis::network::NetworkError&) { errored_.store(true); });
    }

    int connectedCount() const {
        return connectedCount_.load();
    }

    int disconnectedCount() const {
        return disconnectedCount_.load();
    }

    bool disconnectedFlag() const {
        return disconnected_.load();
    }

    bool erroredFlag() const {
        return errored_.load();
    }

    /**
     * @brief Clear the one-shot connected/disconnected/error flags between connections.
     *
     * Reconnect tests must call this after a disconnect so waitForConnection() /
     * waitForDisconnection() observe the second connection rather than stale flags.
     * The cumulative counters (connectedCount etc.) are intentionally preserved.
     */
    void resetConnectionFlags() {
        connected_.store(false);
        disconnected_.store(false);
        errored_.store(false);
    }

private:
    boost::asio::io_context io_;
    TcpTestPeer peer_;
    genesis::network::TcpSession session_;
    std::thread ioThread_;
    bool started_{false};

    std::atomic<bool> connected_{false};
    std::atomic<bool> disconnected_{false};
    std::atomic<bool> errored_{false};
    std::atomic<int> connectedCount_{0};
    std::atomic<int> disconnectedCount_{0};
};

} // namespace genesis::test
