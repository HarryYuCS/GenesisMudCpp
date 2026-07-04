#include <network/telnet_codec.hpp>

#include <gtest/gtest.h>
#include <telnet_bytes.hpp>

using genesis::network::TelnetCodec;
using genesis::test::collectText;
using genesis::test::concat;
using genesis::test::endsWithIacSe;
using genesis::test::extractGmcpBodyFromWire;
using genesis::test::iacWillGmcp;
using genesis::test::iacWillOption;
using genesis::test::sbGmcp;
using genesis::test::startsWithIacDoGmcp;
using genesis::test::startsWithSbGmcp;
using genesis::test::toByte;
using genesis::test::toBytes;

namespace {

constexpr std::uint8_t kNonGmcpOption = 99;

} // namespace

// --- encodeLine ---

TEST(TelnetCodec, EncodeLine_AppendsCrlf) {
    TelnetCodec codec;
    const auto encoded = codec.encodeLine("look");

    ASSERT_EQ(encoded.size(), 6U);
    EXPECT_EQ(encoded[4], toByte('\r'));
    EXPECT_EQ(encoded[5], toByte('\n'));
}

TEST(TelnetCodec, EncodeLine_EmptyString) {
    TelnetCodec codec;
    const auto encoded = codec.encodeLine("");

    ASSERT_EQ(encoded.size(), 2U);
    EXPECT_EQ(encoded[0], toByte('\r'));
    EXPECT_EQ(encoded[1], toByte('\n'));
}

TEST(TelnetCodec, EncodeLine_EscapesEmbeddedIac) {
    TelnetCodec codec;
    const std::string line = std::string("a") + static_cast<char>(255) + "b";
    const auto encoded = codec.encodeLine(line);

    ASSERT_GE(encoded.size(), 5U);
    EXPECT_EQ(encoded[0], toByte('a'));
    EXPECT_EQ(encoded[1], toByte(static_cast<std::uint8_t>(255)));
    EXPECT_EQ(encoded[2], toByte(static_cast<std::uint8_t>(255)));
    EXPECT_EQ(encoded[3], toByte('b'));
    EXPECT_EQ(encoded[encoded.size() - 2], toByte('\r'));
    EXPECT_EQ(encoded[encoded.size() - 1], toByte('\n'));
}

TEST(TelnetCodec, EncodeLine_PreservesNonIacBytes) {
    TelnetCodec codec;
    const auto encoded = codec.encodeLine("say hello");

    ASSERT_EQ(encoded.size(), 11U);
    EXPECT_EQ(encoded[0], toByte('s'));
    EXPECT_EQ(encoded[8], toByte('o'));
}

// --- encodeGmcp ---

TEST(TelnetCodec, EncodeGmcp_WrapsInSbSe) {
    TelnetCodec codec;
    const auto encoded = codec.encodeGmcp(R"({"client":"GenesisCpp"})");

    EXPECT_TRUE(startsWithSbGmcp(encoded));
    EXPECT_TRUE(endsWithIacSe(encoded));
}

TEST(TelnetCodec, EncodeGmcp_DoublesIacInBody) {
    TelnetCodec codec;
    const std::string body = std::string("Core.X ") + static_cast<char>(255) + " tail";
    const auto encoded = codec.encodeGmcp(body);

    ASSERT_TRUE(startsWithSbGmcp(encoded));
    const std::string extracted = extractGmcpBodyFromWire(encoded);
    EXPECT_EQ(extracted, body);
}

TEST(TelnetCodec, EncodeGmcp_PreservesBodyContent) {
    TelnetCodec codec;
    const std::string body = R"(Core.Hello {"client":"GenesisCpp","version":"0.1"})";
    const auto encoded = codec.encodeGmcp(body);

    EXPECT_EQ(extractGmcpBodyFromWire(encoded), body);
}

// --- feed: plain text ---

TEST(TelnetCodec, Feed_PlainTextSingleChunk) {
    TelnetCodec codec;
    const auto result = codec.feed(toBytes("Hello\r\n"));

    ASSERT_EQ(result.textChunks.size(), 1U);
    EXPECT_EQ(result.textChunks[0].text, "Hello\r\n");
    EXPECT_TRUE(result.gmcpPayloads.empty());
    EXPECT_TRUE(result.wireReplies.empty());
}

TEST(TelnetCodec, Feed_PlainTextNoIac) {
    TelnetCodec codec;
    const auto result = codec.feed(toBytes("You are in a forest."));

    ASSERT_EQ(result.textChunks.size(), 1U);
    EXPECT_EQ(result.textChunks[0].text, "You are in a forest.");
    EXPECT_TRUE(result.gmcpPayloads.empty());
}

TEST(TelnetCodec, Feed_IacIacEmitsLiteral255) {
    TelnetCodec codec;
    const std::byte bytes[] = {
        toByte(static_cast<std::uint8_t>(255)),
        toByte(static_cast<std::uint8_t>(255)),
        toByte('A'),
    };
    const auto result = codec.feed(bytes);

    ASSERT_EQ(result.textChunks.size(), 1U);
    EXPECT_EQ(result.textChunks[0].text, std::string(1, static_cast<char>(255)) + "A");
}

TEST(TelnetCodec, Feed_SplitAcrossCalls) {
    TelnetCodec codec;
    std::string allText;
    allText += collectText(codec.feed(toBytes("Hel")));
    allText += collectText(codec.feed(toBytes("lo")));
    EXPECT_EQ(allText, "Hello");
}

TEST(TelnetCodec, Feed_MultipleTextSegments) {
    TelnetCodec codec;
    const auto result = codec.feed(toBytes("segment1\r\nsegment2\r\n"));

    ASSERT_EQ(result.textChunks.size(), 2U);
    EXPECT_EQ(result.textChunks[0].text, "segment1\r\n");
    EXPECT_EQ(result.textChunks[1].text, "segment2\r\n");
}

// --- feed: option negotiation ---

TEST(TelnetCodec, Feed_WillGmcp_ReplyDoGmcp) {
    TelnetCodec codec;
    const auto result = codec.feed(iacWillGmcp());

    EXPECT_TRUE(startsWithIacDoGmcp(result.wireReplies));
    EXPECT_TRUE(result.negotiatedNow);
    EXPECT_TRUE(codec.gmcpEnabled());
}

TEST(TelnetCodec, Feed_WillGmcp_Idempotent) {
    TelnetCodec codec;
    const auto first = codec.feed(iacWillGmcp());
    const auto second = codec.feed(iacWillGmcp());

    EXPECT_TRUE(startsWithIacDoGmcp(first.wireReplies));
    EXPECT_TRUE(second.wireReplies.empty());
    EXPECT_FALSE(second.negotiatedNow);
}

TEST(TelnetCodec, Feed_WillOtherOption_IgnoredOrDeclined) {
    TelnetCodec codec;
    const auto result = codec.feed(iacWillOption(kNonGmcpOption));

    EXPECT_FALSE(codec.gmcpEnabled());
    EXPECT_FALSE(result.negotiatedNow);
}

TEST(TelnetCodec, Feed_NegotiatedNowOnlyOnce) {
    TelnetCodec codec;
    const auto first = codec.feed(iacWillGmcp());
    const auto second = codec.feed(toBytes("ping"));

    EXPECT_TRUE(first.negotiatedNow);
    EXPECT_FALSE(second.negotiatedNow);
}

// --- feed: GMCP subnegotiation ---

TEST(TelnetCodec, Feed_SbGmcp_EmitsPayload) {
    TelnetCodec codec;
    const std::string body = R"(Char.Vitals {"hp":100})";
    const auto result = codec.feed(sbGmcp(body));

    ASSERT_EQ(result.gmcpPayloads.size(), 1U);
    EXPECT_EQ(result.gmcpPayloads[0].body, body);
}

TEST(TelnetCodec, Feed_SbGmcp_SplitAcrossReads) {
    TelnetCodec codec;
    const std::string body = R"(Char.Vitals {"hp":100})";
    const auto frame = sbGmcp(body);
    const auto mid = frame.size() / 2;

    const std::vector<std::byte> first(frame.begin(), frame.begin() + static_cast<std::ptrdiff_t>(mid));
    const std::vector<std::byte> second(frame.begin() + static_cast<std::ptrdiff_t>(mid), frame.end());

    const auto partial = codec.feed(first);
    EXPECT_TRUE(partial.gmcpPayloads.empty());

    const auto complete = codec.feed(second);
    ASSERT_EQ(complete.gmcpPayloads.size(), 1U);
    EXPECT_EQ(complete.gmcpPayloads[0].body, body);
}

TEST(TelnetCodec, Feed_TextThenGmcpSameFeed) {
    TelnetCodec codec;
    const auto result = codec.feed(concat({
        toBytes("Welcome.\r\n"),
        sbGmcp(R"(Room.Info {"id":"start"})"),
    }));

    ASSERT_EQ(result.textChunks.size(), 1U);
    ASSERT_EQ(result.gmcpPayloads.size(), 1U);
    EXPECT_EQ(result.textChunks[0].text, "Welcome.\r\n");
    EXPECT_EQ(result.gmcpPayloads[0].body, R"(Room.Info {"id":"start"})");
}

TEST(TelnetCodec, Feed_GmcpBeforeNegotiation) {
    TelnetCodec codec;
    const std::string body = R"(Char.Vitals {"hp":50})";
    const auto result = codec.feed(sbGmcp(body));

    ASSERT_EQ(result.gmcpPayloads.size(), 1U);
    EXPECT_EQ(result.gmcpPayloads[0].body, body);
    EXPECT_FALSE(codec.gmcpEnabled());
}

// --- reset / gmcpEnabled ---

TEST(TelnetCodec, Reset_ClearsNegotiation) {
    TelnetCodec codec;
    (void)codec.feed(iacWillGmcp());
    ASSERT_TRUE(codec.gmcpEnabled());

    codec.reset();
    EXPECT_FALSE(codec.gmcpEnabled());
}

TEST(TelnetCodec, Reset_AllowsReNegotiation) {
    TelnetCodec codec;
    (void)codec.feed(iacWillGmcp());
    codec.reset();

    const auto result = codec.feed(iacWillGmcp());
    EXPECT_TRUE(startsWithIacDoGmcp(result.wireReplies));
    EXPECT_TRUE(result.negotiatedNow);
    EXPECT_TRUE(codec.gmcpEnabled());
}
