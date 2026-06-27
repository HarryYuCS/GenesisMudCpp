#include <mudcore/gmcp_parser.hpp>

namespace genesis::mudcore {

std::optional<GmcpMessage> GmcpParser::parse(std::string_view raw) const {
    const auto space = raw.find(' ');
    if (space == std::string_view::npos) {
        return GmcpMessage{std::string(raw), ""};
    }

    return GmcpMessage{
        std::string(raw.substr(0, space)),
        std::string(raw.substr(space + 1)),
    };
}

} // namespace genesis::mudcore
