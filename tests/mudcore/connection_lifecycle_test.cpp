#include <mudcore/connection_life_cycle.hpp>

#include <gtest/gtest.h>

#include <string>
#include <vector>

namespace {

using genesis::mudcore::ConnectionLifeCycle;
using genesis::mudcore::ConnectionPhase;

} // namespace

TEST(ConnectionLifeCycle, FullHandshake_SendsHelloAndSupports) {
    std::vector<std::string> sent;
    ConnectionLifeCycle lifecycle([&sent](std::string_view body) {
        sent.emplace_back(body);
    });

    lifecycle.onConnectRequested();
    lifecycle.onTcpConnected();
    lifecycle.onGmcpNegotiated();

    EXPECT_EQ(lifecycle.phase(), ConnectionPhase::HandshakeSent);
    ASSERT_EQ(sent.size(), 2U);
    EXPECT_NE(sent[0].find("Core.Hello"), std::string::npos);
    EXPECT_NE(sent[1].find("Core.Supports.Set"), std::string::npos);
}

TEST(ConnectionLifeCycle, GmcpNegotiated_Idempotent) {
    std::vector<std::string> sent;
    ConnectionLifeCycle lifecycle([&sent](std::string_view body) {
        sent.emplace_back(body);
    });

    lifecycle.onConnectRequested();
    lifecycle.onTcpConnected();
    lifecycle.onGmcpNegotiated();
    lifecycle.onGmcpNegotiated();

    EXPECT_EQ(sent.size(), 2U);
}

TEST(ConnectionLifeCycle, Disconnect_Resets) {
    ConnectionLifeCycle lifecycle([](std::string_view) {});

    lifecycle.onConnectRequested();
    lifecycle.onTcpConnected();
    lifecycle.onTcpDisconnected();

    EXPECT_EQ(lifecycle.phase(), ConnectionPhase::Disconnected);
}

TEST(ConnectionLifeCycle, ConnectRequested_OnlyFromDisconnected) {
    ConnectionLifeCycle lifecycle([](std::string_view) {});

    lifecycle.onConnectRequested();
    EXPECT_EQ(lifecycle.phase(), ConnectionPhase::Connecting);

    lifecycle.onConnectRequested();
    EXPECT_EQ(lifecycle.phase(), ConnectionPhase::Connecting);
}

TEST(ConnectionLifeCycle, TcpConnected_OnlyFromConnecting) {
    ConnectionLifeCycle lifecycle([](std::string_view) {});

    lifecycle.onTcpConnected();
    EXPECT_EQ(lifecycle.phase(), ConnectionPhase::Disconnected);

    lifecycle.onConnectRequested();
    lifecycle.onTcpConnected();
    EXPECT_EQ(lifecycle.phase(), ConnectionPhase::Connected);
}

TEST(ConnectionLifeCycle, GmcpNegotiated_OnlyFromConnected) {
    std::vector<std::string> sent;
    ConnectionLifeCycle lifecycle([&sent](std::string_view body) {
        sent.emplace_back(body);
    });

    lifecycle.onConnectRequested();
    lifecycle.onGmcpNegotiated();
    EXPECT_TRUE(sent.empty());
    EXPECT_EQ(lifecycle.phase(), ConnectionPhase::Connecting);
}

TEST(ConnectionLifeCycle, ReconnectSequence) {
    std::vector<std::string> sent;
    ConnectionLifeCycle lifecycle([&sent](std::string_view body) {
        sent.emplace_back(body);
    });

    lifecycle.onConnectRequested();
    lifecycle.onTcpConnected();
    lifecycle.onGmcpNegotiated();
    lifecycle.onTcpDisconnected();

    lifecycle.onConnectRequested();
    lifecycle.onTcpConnected();
    lifecycle.onGmcpNegotiated();

    EXPECT_EQ(sent.size(), 4U);
    EXPECT_EQ(lifecycle.phase(), ConnectionPhase::HandshakeSent);
}

TEST(ConnectionLifeCycle, InitialPhase_Disconnected) {
    ConnectionLifeCycle lifecycle([](std::string_view) {});
    EXPECT_EQ(lifecycle.phase(), ConnectionPhase::Disconnected);
}

TEST(ConnectionLifeCycle, PlayerLoggedIn_FromHandshakeSent) {
    ConnectionLifeCycle lifecycle([](std::string_view) {});

    lifecycle.onConnectRequested();
    lifecycle.onTcpConnected();
    lifecycle.onGmcpNegotiated();
    ASSERT_EQ(lifecycle.phase(), ConnectionPhase::HandshakeSent);

    lifecycle.onPlayerLoggedIn();
    EXPECT_EQ(lifecycle.phase(), ConnectionPhase::Ready);
}
