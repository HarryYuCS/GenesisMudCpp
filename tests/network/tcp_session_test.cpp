#include <network/tcp_session.hpp>

#include <gtest/gtest.h>
#include <tcp_session_harness.hpp>
#include <telnet_bytes.hpp>

#include <boost/asio.hpp>

#include <atomic>
#include <chrono>
#include <cstddef>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

using genesis::network::ConnectionConfig;
using genesis::network::ConnectionState;
using genesis::network::TcpSession;
using genesis::test::TcpSessionHarness;
using genesis::test::TcpTestPeerConfig;
using genesis::test::TcpTestPeerMode;
using genesis::test::appendSpan;
using genesis::test::bytesToString;
using genesis::test::kDefaultTestTimeout;
using genesis::test::toByte;
using genesis::test::toBytes;
using genesis::test::waitUntil;

namespace {

struct ReadAccumulator {
    mutable std::mutex mutex;
    std::vector<std::byte> bytes;
    std::atomic<int> callCount{0};

    void append(std::span<const std::byte> data) {
        ++callCount;
        const std::lock_guard<std::mutex> lock(mutex);
        appendSpan(bytes, data);
    }

    std::vector<std::byte> snapshot() const {
        const std::lock_guard<std::mutex> lock(mutex);
        return bytes;
    }
};

TcpTestPeerConfig echoPeer() {
    return TcpTestPeerConfig{.mode = TcpTestPeerMode::Echo};
}

TcpTestPeerConfig recordWritesPeer() {
    return TcpTestPeerConfig{.mode = TcpTestPeerMode::RecordWrites};
}

} // namespace

// --- A. Construction and initial state ---

TEST(TcpSession, InitialState_Disconnected) {
    boost::asio::io_context ioContext;
    TcpSession session(ioContext);
    EXPECT_EQ(session.getConnectionState(), ConnectionState::DISCONNECTED);
}

// --- B. Connect — success path (existing + new) ---

TEST(TcpSession, Connect_SucceedsAndInvokesConnectedHandler) {
    TcpSessionHarness harness(echoPeer());
    harness.trackConnected();
    harness.start();
    harness.connectToPeer();

    ASSERT_TRUE(harness.waitForConnection());
    EXPECT_EQ(harness.session().getConnectionState(), ConnectionState::CONNECTED);
    EXPECT_EQ(harness.connectedCount(), 1);

    harness.stop();
}

TEST(TcpSession, Connect_WithoutReadHandler_StillReachesConnected) {
    TcpSessionHarness harness(echoPeer());
    harness.trackConnected();
    harness.start();
    harness.connectToPeer();

    ASSERT_TRUE(harness.waitForConnection());
    EXPECT_EQ(harness.session().getConnectionState(), ConnectionState::CONNECTED);

    harness.stop();
}

TEST(TcpSession, Connect_SecondCallIgnored) {
    TcpSessionHarness harness(echoPeer());
    harness.trackConnected();
    harness.start();
    harness.connectToPeer();

    ASSERT_TRUE(harness.waitForConnection());
    harness.session().connect(ConnectionConfig{"127.0.0.1", harness.peer().port()});

    EXPECT_EQ(harness.connectedCount(), 1);
    EXPECT_EQ(harness.session().getConnectionState(), ConnectionState::CONNECTED);

    harness.stop();
}

// --- C. Connect — failure path ---

TEST(TcpSession, ConnectRefused_InvokesErrorHandler) {
    boost::asio::io_context ioContext;
    TcpSession session(ioContext);

    std::atomic<bool> connected{false};
    std::atomic<bool> errored{false};
    std::atomic<bool> disconnected{false};
    session.setConnectedHandler([&]() { connected.store(true); });
    session.setErrorHandler([&](const genesis::network::NetworkError&) { errored.store(true); });
    session.setDisconnectedHandler([&]() { disconnected.store(true); });
    session.connect(ConnectionConfig{"127.0.0.1", 1});

    std::thread ioThread([&]() { ioContext.run(); });

    ASSERT_TRUE(waitUntil(errored, kDefaultTestTimeout));
    ASSERT_TRUE(waitUntil(disconnected, kDefaultTestTimeout));
    EXPECT_FALSE(connected.load());
    EXPECT_EQ(session.getConnectionState(), ConnectionState::DISCONNECTED);

    ioContext.stop();
    ioThread.join();
}

TEST(TcpSession, Connect_InvalidHost_InvokesError) {
    boost::asio::io_context ioContext;
    TcpSession session(ioContext);

    std::atomic<bool> connected{false};
    std::atomic<bool> errored{false};
    std::atomic<bool> disconnected{false};
    session.setConnectedHandler([&]() { connected.store(true); });
    session.setErrorHandler([&](const genesis::network::NetworkError&) { errored.store(true); });
    session.setDisconnectedHandler([&]() { disconnected.store(true); });
    session.connect(ConnectionConfig{"nonexistent.example.invalid", 80});

    std::thread ioThread([&]() { ioContext.run(); });

    const bool sawFailure = waitUntil([&]() { return errored.load() || disconnected.load(); }, std::chrono::milliseconds(2000));
    if (!sawFailure) {
        GTEST_SKIP() << "DNS did not fail within timeout on this environment";
    }

    if (connected.load() && !errored.load()) {
        GTEST_SKIP() << "Host unexpectedly resolved on this environment";
    }

    EXPECT_TRUE(errored.load());
    EXPECT_TRUE(disconnected.load());
    EXPECT_FALSE(connected.load());
    EXPECT_EQ(session.getConnectionState(), ConnectionState::DISCONNECTED);

    ioContext.stop();
    ioThread.join();
}

// --- D. Disconnect — client-initiated ---

TEST(TcpSession, Disconnect_InvokesDisconnectedHandler) {
    TcpSessionHarness harness(echoPeer());
    harness.trackConnected();
    harness.trackDisconnected();
    harness.start();
    harness.connectToPeer();

    ASSERT_TRUE(harness.waitForConnection());
    harness.session().disconnect();

    ASSERT_TRUE(harness.waitForDisconnection());
    EXPECT_EQ(harness.session().getConnectionState(), ConnectionState::DISCONNECTED);
    EXPECT_EQ(harness.disconnectedCount(), 1);

    harness.stop();
}

TEST(TcpSession, Disconnect_Idempotent) {
    TcpSessionHarness harness(echoPeer());
    harness.trackConnected();
    harness.trackDisconnected();
    harness.start();
    harness.connectToPeer();

    ASSERT_TRUE(harness.waitForConnection());
    harness.session().disconnect();
    ASSERT_TRUE(harness.waitForDisconnection());

    harness.session().disconnect();
    EXPECT_EQ(harness.disconnectedCount(), 1);

    harness.stop();
}

TEST(TcpSession, Disconnect_FromDisconnected_NoOp) {
    TcpSessionHarness harness(echoPeer());
    harness.trackDisconnected();
    harness.start();

    harness.session().disconnect();
    EXPECT_FALSE(harness.disconnectedFlag());
    EXPECT_EQ(harness.session().getConnectionState(), ConnectionState::DISCONNECTED);

    harness.stop();
}

// --- E. Peer-initiated close ---

TEST(TcpSession, PeerClose_InvokesDisconnectedOnly) {
    TcpSessionHarness harness(TcpTestPeerConfig{.mode = TcpTestPeerMode::CloseOnConnect});
    harness.trackDisconnected();
    harness.trackError();
    harness.session().setReadHandler([](std::span<const std::byte>) {});
    harness.start();
    harness.connectToPeer();

    ASSERT_TRUE(harness.waitForDisconnection());
    EXPECT_FALSE(harness.erroredFlag());
    EXPECT_EQ(harness.disconnectedCount(), 1);
    EXPECT_EQ(harness.session().getConnectionState(), ConnectionState::DISCONNECTED);

    harness.stop();
}

TEST(TcpSession, PeerClose_AfterEcho_StillDeliveredData) {
    TcpSessionHarness harness(TcpTestPeerConfig{
        .mode = TcpTestPeerMode::CloseAfterBytes,
        .closeAfterBytes = 2,
    });
    harness.trackConnected();
    harness.trackDisconnected();

    ReadAccumulator reads;
    harness.session().setReadHandler([&](const std::span<const std::byte> data) { reads.append(data); });
    harness.start();
    harness.connectToPeer();

    ASSERT_TRUE(harness.waitForConnection());
    harness.session().send(toBytes("hi"));

    ASSERT_TRUE(waitUntil([&]() { return reads.callCount.load() > 0; }, kDefaultTestTimeout));
    EXPECT_EQ(bytesToString(reads.snapshot()), "hi");

    ASSERT_TRUE(harness.waitForDisconnection());
    EXPECT_EQ(harness.disconnectedCount(), 1);

    harness.stop();
}

// --- F. Send / receive data plane ---

TEST(TcpSession, SendRead_RoundTrip) {
    TcpSessionHarness harness(echoPeer());
    harness.trackConnected();

    ReadAccumulator reads;
    harness.session().setReadHandler([&](const std::span<const std::byte> data) { reads.append(data); });
    harness.start();
    harness.connectToPeer();

    ASSERT_TRUE(harness.waitForConnection());
    harness.session().send(toBytes("hi"));

    ASSERT_TRUE(waitUntil([&]() { return !reads.snapshot().empty(); }, kDefaultTestTimeout));
    EXPECT_EQ(bytesToString(reads.snapshot()), "hi");

    harness.stop();
}

TEST(TcpSession, Send_EmptyPayload_Ignored) {
    TcpSessionHarness harness(recordWritesPeer());
    harness.trackConnected();
    harness.start();
    harness.connectToPeer();

    ASSERT_TRUE(harness.waitForConnection());
    harness.session().send({});

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_TRUE(harness.peer().recordedWrites().empty());

    harness.stop();
}

TEST(TcpSession, Send_WhileDisconnected_Ignored) {
    TcpSessionHarness harness(recordWritesPeer());
    harness.start();

    harness.session().send(toBytes("before"));
    harness.trackConnected();
    harness.connectToPeer();
    ASSERT_TRUE(harness.waitForConnection());

    harness.session().disconnect();
    ASSERT_TRUE(waitUntil([&]() { return harness.session().getConnectionState() == ConnectionState::DISCONNECTED; },
                          kDefaultTestTimeout));

    harness.peer().resetRecordedWrites();
    harness.session().send(toBytes("after"));

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_TRUE(harness.peer().recordedWrites().empty());

    harness.stop();
}

TEST(TcpSession, Read_PushOnConnect) {
    const auto pushPayload = toBytes("pushed");
    TcpSessionHarness harness(TcpTestPeerConfig{
        .mode = TcpTestPeerMode::PushOnConnect,
        .pushPayload = pushPayload,
    });
    harness.trackConnected();

    ReadAccumulator reads;
    harness.session().setReadHandler([&](const std::span<const std::byte> data) { reads.append(data); });
    harness.start();
    harness.connectToPeer();

    ASSERT_TRUE(harness.waitForConnection());
    ASSERT_TRUE(waitUntil([&]() { return reads.callCount.load() > 0; }, kDefaultTestTimeout));
    EXPECT_EQ(reads.snapshot(), pushPayload);

    harness.stop();
}

TEST(TcpSession, Read_MultipleClientSends) {
    TcpSessionHarness harness(echoPeer());
    harness.trackConnected();

    ReadAccumulator reads;
    harness.session().setReadHandler([&](const std::span<const std::byte> data) { reads.append(data); });
    harness.start();
    harness.connectToPeer();

    ASSERT_TRUE(harness.waitForConnection());
    harness.session().send(toBytes("a"));
    harness.session().send(toBytes("b"));
    harness.session().send(toBytes("c"));

    ASSERT_TRUE(waitUntil([&]() { return reads.snapshot().size() >= 3U; }, kDefaultTestTimeout));
    EXPECT_EQ(bytesToString(reads.snapshot()), "abc");

    harness.stop();
}

TEST(TcpSession, Read_LargePayload_MultipleHandlerCalls) {
    TcpSessionHarness harness(echoPeer());
    harness.trackConnected();

    ReadAccumulator reads;
    harness.session().setReadHandler([&](const std::span<const std::byte> data) { reads.append(data); });
    harness.start();
    harness.connectToPeer();

    ASSERT_TRUE(harness.waitForConnection());

    std::vector<std::byte> payload;
    payload.reserve(3000);
    for (int index = 0; index < 3000; ++index) {
        payload.push_back(static_cast<std::byte>(index % 256));
    }

    harness.session().send(payload);

    ASSERT_TRUE(waitUntil([&]() { return reads.snapshot().size() >= payload.size(); }, kDefaultTestTimeout));
    EXPECT_GT(reads.callCount.load(), 1);
    EXPECT_EQ(reads.snapshot(), payload);

    harness.stop();
}

TEST(TcpSession, Read_BinaryPayload_PreservesBytes) {
    TcpSessionHarness harness(echoPeer());
    harness.trackConnected();

    ReadAccumulator reads;
    harness.session().setReadHandler([&](const std::span<const std::byte> data) { reads.append(data); });
    harness.start();
    harness.connectToPeer();

    ASSERT_TRUE(harness.waitForConnection());

    const std::vector<std::byte> payload{
        std::byte{0x00},
        std::byte{0xFF},
        std::byte{0x0D},
        std::byte{0x0A},
        toByte('x'),
    };
    harness.session().send(payload);

    ASSERT_TRUE(waitUntil([&]() { return reads.snapshot().size() >= payload.size(); }, kDefaultTestTimeout));
    EXPECT_EQ(reads.snapshot(), payload);

    harness.stop();
}

// --- G. Reconnect ---

TEST(TcpSession, Reconnect_AfterClientDisconnect) {
    TcpSessionHarness harness(echoPeer());
    harness.trackConnected();
    harness.trackDisconnected();

    ReadAccumulator reads;
    harness.session().setReadHandler([&](const std::span<const std::byte> data) { reads.append(data); });
    harness.start();

    harness.connectToPeer();
    ASSERT_TRUE(harness.waitForConnection());
    harness.session().disconnect();
    ASSERT_TRUE(harness.waitForDisconnection());

    harness.resetConnectionFlags();
    harness.connectToPeer();
    ASSERT_TRUE(harness.waitForConnection());
    EXPECT_EQ(harness.connectedCount(), 2);

    harness.session().send(toBytes("ok"));

    ASSERT_TRUE(waitUntil([&]() { return !reads.snapshot().empty(); }, kDefaultTestTimeout));
    EXPECT_EQ(bytesToString(reads.snapshot()), "ok");

    harness.stop();
}

TEST(TcpSession, Reconnect_AfterPeerClose) {
    TcpSessionHarness harness(TcpTestPeerConfig{
        .mode = TcpTestPeerMode::CloseAfterBytes,
        .closeAfterBytes = 1,
    });
    harness.trackConnected();
    harness.trackDisconnected();

    ReadAccumulator reads;
    harness.session().setReadHandler([&](const std::span<const std::byte> data) { reads.append(data); });
    harness.start();

    harness.connectToPeer();
    ASSERT_TRUE(harness.waitForConnection());
    harness.session().send(toBytes("x"));
    ASSERT_TRUE(harness.waitForDisconnection());

    harness.resetConnectionFlags();
    harness.connectToPeer();
    ASSERT_TRUE(harness.waitForConnection());
    EXPECT_EQ(harness.session().getConnectionState(), ConnectionState::CONNECTED);

    harness.stop();
}

// --- H. Lifetime ---

TEST(TcpSession, DestructorWhileConnected_NotifiesDisconnect) {
    boost::asio::io_context ioContext;
    genesis::test::TcpTestPeer peer(ioContext, echoPeer());

    std::atomic<bool> disconnected{false};
    std::thread ioThread;
    {
        TcpSession session(ioContext);
        session.setDisconnectedHandler([&]() { disconnected.store(true); });
        session.connect(ConnectionConfig{"127.0.0.1", peer.port()});

        ioThread = std::thread([&]() { ioContext.run(); });
        ASSERT_TRUE(waitUntil([&]() { return session.getConnectionState() == ConnectionState::CONNECTED; },
                              kDefaultTestTimeout));
    }

    ASSERT_TRUE(waitUntil(disconnected, kDefaultTestTimeout));
    ioContext.stop();
    ioThread.join();
}

// --- I. Send concurrency ---

// The write queue serializes back-to-back sends, so strict FIFO order is guaranteed.
TEST(TcpSession, Send_BackToBack_BothReachPeerInOrder) {
    TcpSessionHarness harness(recordWritesPeer());
    harness.trackConnected();
    harness.start();
    harness.connectToPeer();

    ASSERT_TRUE(harness.waitForConnection());
    harness.session().send(toBytes("ab"));
    harness.session().send(toBytes("cd"));
    harness.session().send(toBytes("ef"));

    ASSERT_TRUE(waitUntil([&]() { return harness.peer().recordedWrites().size() >= 6U; }, kDefaultTestTimeout));
    EXPECT_EQ(bytesToString(harness.peer().recordedWrites()), "abcdef");

    harness.stop();
}
