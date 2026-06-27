#ifndef NETWORK_TELNET_CODEC_HPP
#define NETWORK_TELNET_CODEC_HPP

#include <string>
#include <string_view>
#include <network/network_types.hpp>
#include <span>
#include <vector>
#include <cstddef>
#include <cstdint>

namespace genesis::network {

// Telnet control codes
constexpr std::uint8_t
    IAC = 255,
    WILL = 251,
    WONT = 252,
    DO = 253,
    DONT = 254,
    SB = 250,
    SE = 240;

// Telnet option codes
constexpr std::uint8_t
    GMCP = 201;

/**
 * @brief The TelnetCodec class is responsible for encoding and decoding Telnet messages.
 *
 */
class TelnetCodec {
public:
    TelnetCodec();
    ~TelnetCodec();

    /**
     * @brief Feeds bytes to the Telnet codec and returns the result.
     *
     * @details Steps the internal state machine based off of the input bytes. Adds replies only for protocol autoreplies
     * 
     * @param bytes The bytes to feed to the Telnet codec.
     * @return The result of the Telnet codec.
    */
    TelnetFeedResult feed(std::span<const std::byte> bytes);
    
    std::vector<std::byte> encodeLine(std::string_view line);
    std::vector<std::byte> encodeGmcp(std::string_view body);

    bool gmcpEnabled() const noexcept;
    void reset();

private:
    enum class ParserState {
        OPTION_NEGOTIATION, // IAC -> WILL <option>
        SUBNEGOTIATION, // IAC -> SB ... IAC -> SE
        PLAIN_TEXT,
    };

    ParserState parserState;
    bool inCommand; // boolean for if last byte was IAC
};

} // namespace genesis::network

#endif // NETWORK_TELNET_CODEC_HPP