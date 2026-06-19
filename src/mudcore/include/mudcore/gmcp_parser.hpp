#ifndef MUDCORE_GMCP_PARSER_HPP
#define MUDCORE_GMCP_PARSER_HPP

#include <string>
#include <network/network_types.hpp>
#include <unordered_map>

namespace genesis::mudcore {

class GMCPParser {
public:
    GMCPParser();
    ~GMCPParser();

    /**
     * @brief Parses a GMCP message and returns a map of the GMCP message data.
     * @param gmcpMessage The GMCP message to parse.
     * @return A map of the GMCP message data.
     */
    std::unordered_map<std::string, std::string> parseGMCPMessage(const genesis::network::GMCPMessage& gmcpMessage) const;
};

} // namespace genesis::mudcore

#endif // MUDCORE_GMCP_PARSER_HPP