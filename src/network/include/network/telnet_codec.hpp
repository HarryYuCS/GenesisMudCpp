#ifndef NETWORK_TELNET_CODEC_HPP
#define NETWORK_TELNET_CODEC_HPP

#include <string>
#include <network/network_types.hpp>

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
     * @details Steps the internal state machine based off of the input bytes
     * 
     * @param bytes The bytes to feed to the Telnet codec.
     * @return The result of the Telnet codec.
    */
    TelnetFeedResult feedBytes(std::span<const std::byte> bytes);
    
    std::vector<std::byte> encodeLine(const std::string_view line);

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