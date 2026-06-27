/**
 * @file network_types.hpp
 * @brief Transport-level types for the network layer.
 *
 * These types must not depend on mudcore or GUI. Session translates them into mudcore::Event.
 */

#ifndef NETWORK_NETWORK_TYPES_HPP
#define NETWORK_NETWORK_TYPES_HPP

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace genesis::network {

/**
 * @brief TCP socket lifecycle state.
 */
enum class ConnectionState {
    DISCONNECTED, ///< Socket is closed.
    RESOLVING,    ///< DNS resolution in progress.
    CONNECTING,   ///< TCP handshake in progress.
    CONNECTED,    ///< Socket is open and ready.
};

/** @brief Host and port used to open a TCP connection. */
struct ConnectionConfig {
    std::string host;
    std::uint16_t port;
};

/** @brief Error reported from async network operations. */
struct NetworkError {
    std::string errorMessage;
};

/**
 * @brief Raw GMCP body extracted from IAC SB GMCP ... IAC SE.
 *
 * Format: "Package.Name {...}" — JSON parsing happens in mudcore.
 */
struct GmcpPayload {
    std::string body;
};

/**
 * @brief Player-visible text outside telnet subnegotiation frames.
 *
 * IAC control sequences have been stripped by TelnetCodec.
 */
struct MudTextChunk {
    std::string text;
};

/**
 * @brief Output of TelnetCodec::feed() for one chunk of received bytes.
 */
struct TelnetFeedResult {
    std::vector<MudTextChunk> textChunks;  ///< Decoded MUD text for display pipeline.
    std::vector<GmcpPayload> gmcpPayloads; ///< Decoded GMCP bodies for mudcore parser.
    std::vector<std::byte> wireReplies;  ///< Protocol auto-replies (e.g. IAC DO GMCP); send immediately.

    /** @brief true if GMCP was just negotiated this feed(); triggers Core handshake in Session. */
    bool negotiatedNow = false;
};

} // namespace genesis::network

#endif // NETWORK_NETWORK_TYPES_HPP
