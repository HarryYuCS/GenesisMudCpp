#pragma once

#include <network/telnet_codec.hpp>

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace genesis::test {

inline std::byte toByte(std::uint8_t value) {
    return static_cast<std::byte>(value);
}

inline std::byte toByte(char ch) {
    return toByte(static_cast<std::uint8_t>(static_cast<unsigned char>(ch)));
}

inline std::vector<std::byte> toBytes(std::string_view text) {
    std::vector<std::byte> bytes;
    bytes.reserve(text.size());
    for (char ch : text) {
        bytes.push_back(toByte(ch));
    }
    return bytes;
}

inline std::vector<std::byte> concat(std::initializer_list<std::vector<std::byte>> parts) {
    std::vector<std::byte> combined;
    for (const auto& part : parts) {
        combined.insert(combined.end(), part.begin(), part.end());
    }
    return combined;
}

inline std::vector<std::byte> iacWillGmcp() {
    using genesis::network::GMCP;
    using genesis::network::IAC;
    using genesis::network::WILL;
    return {toByte(IAC), toByte(WILL), toByte(GMCP)};
}

inline std::vector<std::byte> iacWillOption(std::uint8_t option) {
    using genesis::network::IAC;
    using genesis::network::WILL;
    return {toByte(IAC), toByte(WILL), toByte(option)};
}

inline std::vector<std::byte> sbGmcp(std::string_view body) {
    using genesis::network::GMCP;
    using genesis::network::IAC;
    using genesis::network::SB;
    using genesis::network::SE;

    std::vector<std::byte> frame;
    frame.reserve(body.size() + 6);
    frame.push_back(toByte(IAC));
    frame.push_back(toByte(SB));
    frame.push_back(toByte(GMCP));
    for (char ch : body) {
        const auto byte = static_cast<std::uint8_t>(static_cast<unsigned char>(ch));
        frame.push_back(toByte(byte));
        if (byte == IAC) {
            frame.push_back(toByte(IAC));
        }
    }
    frame.push_back(toByte(IAC));
    frame.push_back(toByte(SE));
    return frame;
}

inline bool startsWithIacDoGmcp(const std::vector<std::byte>& wire) {
    using genesis::network::DO;
    using genesis::network::GMCP;
    using genesis::network::IAC;
    return wire.size() >= 3
        && wire[0] == toByte(IAC)
        && wire[1] == toByte(DO)
        && wire[2] == toByte(GMCP);
}

inline bool startsWithSbGmcp(const std::vector<std::byte>& wire) {
    using genesis::network::GMCP;
    using genesis::network::IAC;
    using genesis::network::SB;
    return wire.size() >= 3
        && wire[0] == toByte(IAC)
        && wire[1] == toByte(SB)
        && wire[2] == toByte(GMCP);
}

inline bool endsWithIacSe(const std::vector<std::byte>& wire) {
    using genesis::network::IAC;
    using genesis::network::SE;
    return wire.size() >= 2
        && wire[wire.size() - 2] == toByte(IAC)
        && wire[wire.size() - 1] == toByte(SE);
}

/** Extract GMCP body bytes between IAC SB GMCP and IAC SE (undoubles IAC). */
inline std::string extractGmcpBodyFromWire(const std::vector<std::byte>& wire) {
    using genesis::network::IAC;

    if (!startsWithSbGmcp(wire) || !endsWithIacSe(wire)) {
        return {};
    }

    std::string body;
    for (std::size_t i = 3; i + 1 < wire.size(); ++i) {
        const auto byte = static_cast<std::uint8_t>(wire[i]);
        if (byte == IAC && static_cast<std::uint8_t>(wire[i + 1]) == IAC) {
            body.push_back(static_cast<char>(IAC));
            ++i;
            continue;
        }
        if (byte == IAC && static_cast<std::uint8_t>(wire[i + 1]) == genesis::network::SE) {
            break;
        }
        body.push_back(static_cast<char>(byte));
    }
    return body;
}

inline std::string collectText(const genesis::network::TelnetFeedResult& result) {
    std::string combined;
    for (const auto& chunk : result.textChunks) {
        combined += chunk.text;
    }
    return combined;
}

} // namespace genesis::test
