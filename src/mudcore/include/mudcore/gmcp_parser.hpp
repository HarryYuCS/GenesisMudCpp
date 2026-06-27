/**
 * @file gmcp_parser.hpp
 * @brief Parses GMCP package names and JSON bodies from raw telnet payloads.
 */

#ifndef MUDCORE_GMCP_PARSER_HPP
#define MUDCORE_GMCP_PARSER_HPP

#include <optional>
#include <string>
#include <string_view>

namespace genesis::mudcore {

/**
 * @brief Parsed GMCP message with package name and JSON payload.
 */
struct GmcpMessage {
    std::string package;  ///< e.g. "Char.Vitals", "Room.Info".
    std::string jsonBody;   ///< JSON object or array as a string; may be empty.
};

/**
 * @brief Splits a raw GMCP body into package name and JSON.
 */
class GmcpParser {
public:
    /**
     * @brief Parse a GMCP payload of the form "Package.Name {...}".
     *
     * @param raw Full GMCP body from telnet subnegotiation (no IAC framing).
     * @return Parsed message, or a package-only message if no space separator is found.
     */
    std::optional<GmcpMessage> parse(std::string_view raw) const;
};

} // namespace genesis::mudcore

#endif // MUDCORE_GMCP_PARSER_HPP
