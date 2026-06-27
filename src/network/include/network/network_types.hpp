#ifndef NETWORK_NETWORK_TYPES_HPP
#define NETWORK_NETWORK_TYPES_HPP

#include <string>
#include <vector>
#include <cstddef>
#include <cstdint>

namespace genesis::network {

enum class ConnectionState {
    DISCONNECTED,
    RESOLVING,
    CONNECTING,
    CONNECTED,
};

struct ConnectionConfig { std::string host; uint16_t port; };

struct NetworkError { std::string errorMessage; };

struct GmcpPayload { std::string body; };
struct MudTextChunk { std::string text; };

struct TelnetFeedResult {
    std::vector<MudTextChunk> textChunks;
    std::vector<GmcpPayload> gmcpPayloads;
    std::vector<std::byte> wireReplies; // send immediately

    bool negotiatedNow = false;
};

} // namespace genesis::network

#endif // NETWORK_NETWORK_TYPES_HPP