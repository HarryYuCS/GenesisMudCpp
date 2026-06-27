#include <network/telnet_codec.hpp>

namespace genesis::network {

TelnetCodec::TelnetCodec() = default;
TelnetCodec::~TelnetCodec() = default;

TelnetFeedResult TelnetCodec::feed(std::span<const std::byte> bytes) {
    (void)bytes;
    return {};
}

std::vector<std::byte> TelnetCodec::encodeLine(std::string_view line) {
    std::vector<std::byte> encoded;
    encoded.reserve(line.size() + 2);
    for (char ch : line) {
        encoded.push_back(static_cast<std::byte>(ch));
    }
    encoded.push_back(static_cast<std::byte>('\r'));
    encoded.push_back(static_cast<std::byte>('\n'));
    return encoded;
}

std::vector<std::byte> TelnetCodec::encodeGmcp(std::string_view body) {
    std::vector<std::byte> encoded;
    encoded.reserve(body.size());
    for (char ch : body) {
        encoded.push_back(static_cast<std::byte>(ch));
    }
    return encoded;
}

bool TelnetCodec::gmcpEnabled() const noexcept {
    return false;
}

void TelnetCodec::reset() {}

} // namespace genesis::network
