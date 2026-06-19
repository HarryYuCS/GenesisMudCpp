#ifndef NETWORK_NETWORK_TYPES_HPP
#define NETWORK_NETWORK_TYPES_HPP

#include <string>

namespace genesis::network {

struct NetworkError { std::string errorMessage; };

struct GMCPMessage { std::string message; };
struct MudTextChunk { std::string text; }; // no IAC control codes, inside SB

/**
 * @brief Stores the result of a Telnet feed, with the additional wireReplies member for suggesting immediate replies
 *
 */
struct TelnetFeedResult {
    std::vector<MudTextChunk> textChunks;
    std::vector<GMCPMessage> gmcpMessages;
    std::vector<std::byte> wireReplies; // send immediately
};

} // namespace genesis::network

#endif // NETWORK_NETWORK_TYPES_HPP