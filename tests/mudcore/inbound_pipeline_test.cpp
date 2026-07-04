#include <mudcore/inbound_pipeline.hpp>

#include <gtest/gtest.h>
#include <gmcp_samples.hpp>

namespace {

using genesis::mudcore::GameState;
using genesis::mudcore::InboundPipeline;
using genesis::mudcore::OutputSink;
using genesis::test::charLoginJson;
using genesis::test::charVitalsRaw;
using genesis::test::commChannelJson;
using genesis::test::commChannelRaw;
using genesis::test::rawGmcp;
using genesis::test::roomInfoRaw;
using genesis::test::unknownPackageRaw;

} // namespace

TEST(InboundPipeline, ProcessMudText_MainSink) {
    InboundPipeline pipeline;
    const auto line = pipeline.processMudText("You see a tree.");

    ASSERT_TRUE(line.has_value());
    EXPECT_EQ(line->sink, OutputSink::Main);
    EXPECT_EQ(line->text, "You see a tree.");
}

TEST(InboundPipeline, ProcessMudText_Empty) {
    InboundPipeline pipeline;
    EXPECT_FALSE(pipeline.processMudText("").has_value());
}

TEST(InboundPipeline, ProcessMudText_PreservesContent) {
    InboundPipeline pipeline;
    const auto line = pipeline.processMudText("line one\nline two");

    ASSERT_TRUE(line.has_value());
    EXPECT_EQ(line->text, "line one\nline two");
}

TEST(InboundPipeline, ProcessGmcp_Comm_RoutesToComms) {
    InboundPipeline pipeline;
    GameState state;
    const auto result = pipeline.processGmcp(commChannelRaw(), state);

    ASSERT_TRUE(result.line.has_value());
    EXPECT_EQ(result.line->sink, OutputSink::Comms);
    EXPECT_EQ(result.line->text, "You say: How are you?");
    EXPECT_FALSE(result.stateChanged);
}

TEST(InboundPipeline, ProcessGmcp_Comm_UsesMessageField) {
    InboundPipeline pipeline;
    GameState state;
    const auto result = pipeline.processGmcp(commChannelRaw(), state);

    ASSERT_TRUE(result.line.has_value());
    EXPECT_NE(result.line->text, commChannelJson());
    EXPECT_EQ(result.line->text, "You say: How are you?");
}

TEST(InboundPipeline, ProcessGmcp_RoomInfo_UpdatesState) {
    InboundPipeline pipeline;
    GameState state;
    const auto result = pipeline.processGmcp(roomInfoRaw(), state);

    EXPECT_FALSE(result.line.has_value());
    EXPECT_TRUE(result.stateChanged);
    EXPECT_EQ(state.room().roomId, "iX5PnH");
}

TEST(InboundPipeline, ProcessGmcp_CharVitals_UpdatesStateNoLines) {
    InboundPipeline pipeline;
    GameState state;
    const auto result = pipeline.processGmcp(charVitalsRaw(), state);

    EXPECT_FALSE(result.line.has_value());
    EXPECT_TRUE(result.stateChanged);
    EXPECT_EQ(state.healthLevel(), "very hurt");
}

TEST(InboundPipeline, ProcessGmcp_CharVitals_PartialMergeThroughPipeline) {
    InboundPipeline pipeline;
    GameState state;

    pipeline.processGmcp(charVitalsRaw(), state);

    const auto result = pipeline.processGmcp(R"(Char.Vitals {"mana":"depleted"})", state);
    EXPECT_TRUE(result.stateChanged);
    EXPECT_EQ(state.healthLevel(), "very hurt");
    EXPECT_EQ(state.manaLevel(), "depleted");
}

TEST(InboundPipeline, ProcessGmcp_UnknownPackage_NoLines) {
    InboundPipeline pipeline;
    GameState state;
    state.setVariable("marker", "unchanged");
    const auto result = pipeline.processGmcp(unknownPackageRaw(), state);

    EXPECT_FALSE(result.line.has_value());
    EXPECT_FALSE(result.stateChanged);
    EXPECT_EQ(state.getVariable("marker"), "unchanged");
}

TEST(InboundPipeline, ProcessGmcp_CharVitals_NoStateChangedWhenUnchanged) {
    InboundPipeline pipeline;
    GameState state;
    ASSERT_TRUE(pipeline.processGmcp(charVitalsRaw(), state).stateChanged);

    const auto result = pipeline.processGmcp(charVitalsRaw(), state);
    EXPECT_FALSE(result.stateChanged);
}

TEST(InboundPipeline, ProcessGmcp_CharLogin_ReportsLogin) {
    InboundPipeline pipeline;
    GameState state;
    const auto result = pipeline.processGmcp(rawGmcp("Char.Login", charLoginJson()), state);

    EXPECT_TRUE(result.playerLoggedIn);
    EXPECT_TRUE(result.stateChanged);
    EXPECT_FALSE(result.line.has_value());
    EXPECT_TRUE(state.loggedIn());
}

TEST(InboundPipeline, ProcessGmcp_NonLoginPackages_DoNotReportLogin) {
    InboundPipeline pipeline;
    GameState state;

    EXPECT_FALSE(pipeline.processGmcp(charVitalsRaw(), state).playerLoggedIn);
    EXPECT_FALSE(pipeline.processGmcp(roomInfoRaw(), state).playerLoggedIn);
    EXPECT_FALSE(pipeline.processGmcp(commChannelRaw(), state).playerLoggedIn);
    EXPECT_FALSE(pipeline.processGmcp(unknownPackageRaw(), state).playerLoggedIn);
}
