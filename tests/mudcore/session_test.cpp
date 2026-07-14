#include <mudcore/session.hpp>

#include <gtest/gtest.h>
#include <gmcp_samples.hpp>
#include <tcp_test_peer.hpp>
#include <telnet_bytes.hpp>

#include <boost/asio.hpp>

#include <algorithm>
#include <chrono>
#include <functional>
#include <string>
#include <thread>
#include <vector>

using genesis::mudcore::ConnectionPhase;
using genesis::mudcore::DisplayLine;
using genesis::mudcore::OutputSink;
using genesis::mudcore::Session;
using genesis::test::charLoginJson;
using genesis::test::concat;
using genesis::test::iacWillGmcp;
using genesis::test::rawGmcp;
using genesis::test::sbGmcp;
using genesis::test::TcpTestPeer;
using genesis::test::TcpTestPeerConfig;
using genesis::test::TcpTestPeerMode;

namespace {

constexpr std::chrono::milliseconds kPollTimeout{1000};

/**
 * @brief Runs a Session against a scripted loopback peer, polling on the test thread.
 */
class SessionFixture {
public:
    explicit SessionFixture(TcpTestPeerConfig peerConfig = {})
        : peer_(io_, std::move(peerConfig))
        , session_(io_)
        , ioThread_([this]() { io_.run(); }) {}

    ~SessionFixture() {
        session_.disconnect();
        io_.stop();
        ioThread_.join();
    }

    Session& session() {
        return session_;
    }

    void connect() {
        session_.connect("127.0.0.1", peer_.port());
    }

    /** @brief Poll until the predicate is satisfied; accumulates lines across polls. */
    bool pollUntil(const std::function<bool()>& predicate) {
        const auto deadline = std::chrono::steady_clock::now() + kPollTimeout;
        while (std::chrono::steady_clock::now() < deadline) {
            auto result = session_.poll();
            for (auto& line : result.lines) {
                lines_.push_back(std::move(line));
            }
            if (predicate()) {
                return true;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        return predicate();
    }

    bool hasSystemLine(const std::string& text) const {
        return std::any_of(lines_.begin(), lines_.end(), [&](const DisplayLine& line) {
            return line.sink == OutputSink::System && line.text == text;
        });
    }

    bool hasSystemLineContaining(const std::string& fragment) const {
        return std::any_of(lines_.begin(), lines_.end(), [&](const DisplayLine& line) {
            return line.sink == OutputSink::System && line.text.find(fragment) != std::string::npos;
        });
    }

    void clearAccumulatedLines() {
        lines_.clear();
    }

private:
    boost::asio::io_context io_;
    TcpTestPeer peer_;
    Session session_;
    std::thread ioThread_;
    std::vector<DisplayLine> lines_;
};

} // namespace

TEST(Session, Poll_ConnectEmitsSystemLineAndPhase) {
    SessionFixture fixture;
    fixture.connect();

    ASSERT_TRUE(fixture.pollUntil([&]() { return fixture.hasSystemLine("Connected."); }));
    EXPECT_EQ(fixture.session().connectionPhase(), ConnectionPhase::Connected);
}

TEST(Session, Poll_GmcpNegotiated_DrivesHandshake) {
    SessionFixture fixture(TcpTestPeerConfig{
        .mode = TcpTestPeerMode::PushOnConnect,
        .pushPayload = iacWillGmcp(),
    });
    fixture.connect();

    ASSERT_TRUE(fixture.pollUntil(
        [&]() { return fixture.session().connectionPhase() == ConnectionPhase::HandshakeSent; }));
}

TEST(Session, Poll_CharLogin_DrivesReadyPhase) {
    // Push GMCP negotiation followed by a Char.Login broadcast in one payload:
    // negotiation -> HandshakeSent, then the login message -> Ready.
    SessionFixture fixture(TcpTestPeerConfig{
        .mode = TcpTestPeerMode::PushOnConnect,
        .pushPayload = concat({iacWillGmcp(), sbGmcp(rawGmcp("Char.Login", charLoginJson()))}),
    });
    fixture.connect();

    ASSERT_TRUE(fixture.pollUntil(
        [&]() { return fixture.session().connectionPhase() == ConnectionPhase::Ready; }));
    EXPECT_TRUE(fixture.session().gameState().loggedIn());
}

TEST(Session, Poll_Reconnect_ReachesConnected) {
    SessionFixture fixture;
    fixture.connect();
    ASSERT_TRUE(fixture.pollUntil([&]() { return fixture.hasSystemLine("Connected."); }));

    fixture.session().disconnect();
    ASSERT_TRUE(fixture.pollUntil([&]() { return fixture.hasSystemLine("Disconnected."); }));
    EXPECT_FALSE(fixture.session().gameState().loggedIn());

    fixture.clearAccumulatedLines();
    fixture.connect();
    ASSERT_TRUE(fixture.pollUntil([&]() { return fixture.hasSystemLine("Connected."); }));
    EXPECT_GE(fixture.session().connectionPhase(), ConnectionPhase::Connected);
}

TEST(Session, Poll_DisconnectResetsGameState) {
    SessionFixture fixture(TcpTestPeerConfig{
        .mode = TcpTestPeerMode::PushOnConnect,
        .pushPayload = concat({iacWillGmcp(), sbGmcp(rawGmcp("Char.Login", charLoginJson()))}),
    });
    fixture.connect();
    ASSERT_TRUE(fixture.pollUntil(
        [&]() { return fixture.session().connectionPhase() == ConnectionPhase::Ready; }));
    EXPECT_TRUE(fixture.session().gameState().loggedIn());

    fixture.session().disconnect();
    EXPECT_FALSE(fixture.session().gameState().loggedIn());
    EXPECT_TRUE(fixture.session().gameState().playerName().empty());
}

TEST(Session, Poll_DisconnectEmitsSystemLine) {
    SessionFixture fixture;
    fixture.connect();
    ASSERT_TRUE(fixture.pollUntil([&]() { return fixture.hasSystemLine("Connected."); }));

    fixture.session().disconnect();

    ASSERT_TRUE(fixture.pollUntil([&]() { return fixture.hasSystemLine("Disconnected."); }));
    EXPECT_EQ(fixture.session().connectionPhase(), ConnectionPhase::Disconnected);
}

TEST(Session, Poll_ConnectionRefusedEmitsErrorLine) {
    boost::asio::io_context io;
    Session session(io);
    session.connect("127.0.0.1", 1);
    std::thread ioThread([&]() { io.run(); });

    std::vector<DisplayLine> lines;
    const auto deadline = std::chrono::steady_clock::now() + kPollTimeout;
    bool sawError = false;
    while (std::chrono::steady_clock::now() < deadline && !sawError) {
        auto result = session.poll();
        for (auto& line : result.lines) {
            lines.push_back(std::move(line));
        }
        sawError = std::any_of(lines.begin(), lines.end(), [](const DisplayLine& line) {
            return line.sink == OutputSink::System && line.text.find("Network error:") == 0;
        });
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    EXPECT_TRUE(sawError);
    EXPECT_EQ(session.connectionPhase(), ConnectionPhase::Disconnected);

    io.stop();
    ioThread.join();
}
