#include <mudcore/game_state.hpp>

#include <gtest/gtest.h>
#include <gmcp_samples.hpp>

namespace {

using genesis::mudcore::GameState;
using genesis::mudcore::GmcpMessage;
using genesis::test::charLoginMessage;
using genesis::test::charVitalsMessage;
using genesis::test::roomInfoMessage;
using genesis::test::roomMapMessage;
using genesis::test::unknownPackageMessage;

} // namespace

TEST(GameState, Room_InitialEmpty) {
    GameState state;
    EXPECT_TRUE(state.room().roomId.empty());
    EXPECT_TRUE(state.room().shortDescription.empty());
    EXPECT_TRUE(state.room().exits.empty());
    EXPECT_FALSE(state.loggedIn());
}

TEST(GameState, Variables_SetGet) {
    GameState state;
    state.setVariable("foo", "bar");
    EXPECT_EQ(state.getVariable("foo"), "bar");
}

TEST(GameState, Variables_MissingKey_EmptyString) {
    GameState state;
    EXPECT_TRUE(state.getVariable("missing").empty());
}

TEST(GameState, ApplyGmcp_UnknownPackage_NoThrow) {
    GameState before;
    before.setVariable("keep", "yes");

    GameState after;
    after.setVariable("keep", "yes");
    after.applyGmcp(unknownPackageMessage());

    EXPECT_EQ(after.getVariable("keep"), "yes");
    EXPECT_EQ(after.healthLevel(), before.healthLevel());
}

TEST(GameState, ApplyGmcp_CharVitals_UpdatesHealth) {
    GameState state;
    auto message = charVitalsMessage();
    message.jsonBody = R"({"health":"death's door"})";
    state.applyGmcp(message);
    EXPECT_EQ(state.healthLevel(), "death's door");
}

TEST(GameState, ApplyGmcp_CharVitals_UpdatesMana) {
    GameState state;
    auto message = charVitalsMessage();
    message.jsonBody = R"({"mana":"low"})";
    state.applyGmcp(message);
    EXPECT_EQ(state.manaLevel(), "low");
}

TEST(GameState, ApplyGmcp_CharVitals_UpdatesAllVitals) {
    GameState state;
    state.applyGmcp(charVitalsMessage());

    EXPECT_EQ(state.healthLevel(), "very hurt");
    EXPECT_EQ(state.manaLevel(), "in full vigour");
    EXPECT_EQ(state.foodLevel(), "satisfied");
    EXPECT_EQ(state.drinkLevel(), "quenched");
    EXPECT_EQ(state.fatigueLevel(), "rested");
    EXPECT_EQ(state.intoxicationLevel(), "sober");
}

TEST(GameState, ApplyGmcp_CharVitals_PartialMerge) {
    GameState state;
    state.applyGmcp(charVitalsMessage());

    auto delta = charVitalsMessage();
    delta.jsonBody = R"({"health":"death's door"})";
    state.applyGmcp(delta);

    EXPECT_EQ(state.healthLevel(), "death's door");
    EXPECT_EQ(state.manaLevel(), "in full vigour");
    EXPECT_EQ(state.foodLevel(), "satisfied");
}

TEST(GameState, ApplyGmcp_CharVitals_CaseInsensitivePackage) {
    GameState state;
    GmcpMessage message{
        "char.vitals",
        R"({"health":"very hurt"})",
    };
    state.applyGmcp(message);
    EXPECT_EQ(state.healthLevel(), "very hurt");
}

TEST(GameState, ApplyGmcp_RoomInfo_UpdatesRoom) {
    GameState state;
    state.applyGmcp(roomInfoMessage());

    EXPECT_EQ(state.room().roomId, "iX5PnH");
    EXPECT_EQ(state.room().shortDescription, "A busy plaza.");
    ASSERT_EQ(state.room().exits.size(), 2U);
    EXPECT_EQ(state.room().exits[0], "north");
    EXPECT_EQ(state.room().exits[1], "east");
}

TEST(GameState, ApplyGmcp_RoomInfo_ParsesDoorsAndCoords) {
    GameState state;
    state.applyGmcp(roomInfoMessage());

    ASSERT_EQ(state.room().doors.size(), 1U);
    EXPECT_EQ(state.room().doors[0], "north");
    ASSERT_TRUE(state.room().x.has_value());
    ASSERT_TRUE(state.room().y.has_value());
    EXPECT_EQ(*state.room().x, 10);
    EXPECT_EQ(*state.room().y, 20);
}

TEST(GameState, ApplyGmcp_RoomMap_UpdatesMapGraphics) {
    GameState state;
    state.applyGmcp(roomMapMessage());

    EXPECT_EQ(state.room().map, "map graphics");
    EXPECT_EQ(state.room().zoom, "zoomed map graphics");
}

TEST(GameState, ApplyGmcp_CharLogin_SetsLoggedIn) {
    GameState state;
    state.applyGmcp(charLoginMessage());
    EXPECT_TRUE(state.loggedIn());
    EXPECT_EQ(state.playerName(), "eowul");
}

TEST(GameState, ApplyGmcp_CharLogin_RequiresName) {
    GameState state;
    state.applyGmcp(GmcpMessage{"Char.Login", R"({"uid":12345})"});
    EXPECT_FALSE(state.loggedIn());
    EXPECT_TRUE(state.playerName().empty());
}

TEST(GameState, ApplyGmcp_ReturnsFalseWhenUnchanged) {
    GameState state;
    ASSERT_TRUE(state.applyGmcp(charVitalsMessage()));
    EXPECT_FALSE(state.applyGmcp(charVitalsMessage()));
}
