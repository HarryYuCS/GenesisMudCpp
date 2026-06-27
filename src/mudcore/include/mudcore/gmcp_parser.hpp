#ifndef MUDCORE_GMCP_PARSER_HPP
#define MUDCORE_GMCP_PARSER_HPP

#include <optional>
#include <string>
#include <string_view>

namespace genesis::mudcore {

struct GmcpMessage {
    std::string package;
    std::string jsonBody;
};

class GmcpParser {
public:
    /** @brief Split "Package.Name {...}" into package + JSON body. */
    std::optional<GmcpMessage> parse(std::string_view raw) const;
};

} // namespace genesis::mudcore

#endif // MUDCORE_GMCP_PARSER_HPP
