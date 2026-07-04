#include <mudcore/gmcp_parser.hpp>

#include <gtest/gtest.h>
#include <gmcp_samples.hpp>

TEST(GmcpParser, Parse_SplitsPackageAndJson) {
    genesis::mudcore::GmcpParser parser;
    const auto msg = parser.parse(R"(Char.Vitals {"hp":100})");

    ASSERT_TRUE(msg.has_value());
    EXPECT_EQ(msg->package, "Char.Vitals");
    EXPECT_EQ(msg->jsonBody, R"({"hp":100})");
}

TEST(GmcpParser, Parse_PackageOnly) {
    genesis::mudcore::GmcpParser parser;
    const auto msg = parser.parse("Core.Hello");

    ASSERT_TRUE(msg.has_value());
    EXPECT_EQ(msg->package, "Core.Hello");
    EXPECT_TRUE(msg->jsonBody.empty());
}

TEST(GmcpParser, Parse_EmptyString) {
    genesis::mudcore::GmcpParser parser;
    EXPECT_FALSE(parser.parse("").has_value());
}

TEST(GmcpParser, Parse_OnlyFirstSpaceSplits) {
    genesis::mudcore::GmcpParser parser;
    const auto msg = parser.parse("A.B extra json");

    ASSERT_TRUE(msg.has_value());
    EXPECT_EQ(msg->package, "A.B");
    EXPECT_EQ(msg->jsonBody, "extra json");
}

TEST(GmcpParser, Parse_CoreSupportsSetArray) {
    genesis::mudcore::GmcpParser parser;
    const std::string raw = std::string("Core.Supports.Set ") + std::string(genesis::test::coreSupportsSetJson());
    const auto msg = parser.parse(raw);

    ASSERT_TRUE(msg.has_value());
    EXPECT_EQ(msg->package, "Core.Supports.Set");
    EXPECT_EQ(msg->jsonBody, genesis::test::coreSupportsSetJson());
}

TEST(GmcpParser, Parse_RoomInfoObject) {
    genesis::mudcore::GmcpParser parser;
    const std::string raw = std::string("Room.Info ") + std::string(genesis::test::roomInfoJson());
    const auto msg = parser.parse(raw);

    ASSERT_TRUE(msg.has_value());
    EXPECT_EQ(msg->package, "Room.Info");
    EXPECT_EQ(msg->jsonBody, genesis::test::roomInfoJson());
}

TEST(GmcpParser, Parse_LeadingTrailingSpace) {
    genesis::mudcore::GmcpParser parser;
    const auto msg = parser.parse(" Char.Vitals  {\"hp\":1} ");

    ASSERT_TRUE(msg.has_value());
    EXPECT_EQ(msg->package, "Char.Vitals");
    EXPECT_EQ(msg->jsonBody, "{\"hp\":1}");
}

TEST(GmcpParser, Parse_NoJsonAfterSpace) {
    genesis::mudcore::GmcpParser parser;
    const auto msg = parser.parse("Char.Vitals ");

    ASSERT_TRUE(msg.has_value());
    EXPECT_EQ(msg->package, "Char.Vitals");
    EXPECT_TRUE(msg->jsonBody.empty());
}
