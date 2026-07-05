#include <mudcore/gmcp_parser.hpp>

#include <cctype>

namespace genesis::mudcore {

namespace {

/** @brief Check if a character is an ASCII space. */
bool isAsciiSpace(const char ch) {
    return std::isspace(static_cast<unsigned char>(ch)) != 0;
}

/** @brief Trim leading and trailing ASCII spaces from a string view. */
std::string_view trimView(std::string_view text) {
    while (!text.empty() && isAsciiSpace(text.front())) {
        text.remove_prefix(1);
    }
    while (!text.empty() && isAsciiSpace(text.back())) {
        text.remove_suffix(1);
    }
    return text;
}

} // namespace

/** @brief Parse a GMCP message object from a string view. */
std::optional<GmcpMessage> GmcpParser::parse(const std::string_view raw) const {
    const std::string_view trimmed = trimView(raw);
    if (trimmed.empty()) {
        return std::nullopt;
    }

    const auto space = trimmed.find(' ');
    if (space == std::string_view::npos) {
        return GmcpMessage{std::string(trimmed), ""};
    }

    // return the package and json body
    return GmcpMessage{
        std::string(trimView(trimmed.substr(0, space))),
        std::string(trimView(trimmed.substr(space + 1))),
    };
}

} // namespace genesis::mudcore
