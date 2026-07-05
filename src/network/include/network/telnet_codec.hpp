/**
 * @file telnet_codec.hpp
 * @brief Stateful telnet parser and encoder for MUD connections.
 *
 * Separates wire protocol (IAC, SB/SE, option negotiation) from application data (text, GMCP).
 */

#ifndef NETWORK_TELNET_CODEC_HPP
#define NETWORK_TELNET_CODEC_HPP

#include <network/network_types.hpp>

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace genesis::network {

/** @brief Telnet Interpret As Command byte. */
constexpr std::uint8_t IAC = 255;

constexpr std::uint8_t WILL = 251;
constexpr std::uint8_t WONT = 252;
constexpr std::uint8_t DO = 253;
constexpr std::uint8_t DONT = 254;
constexpr std::uint8_t SB = 250;
constexpr std::uint8_t SE = 240;

/** @brief Telnet option code for Generic MUD Communication Protocol. */
constexpr std::uint8_t GMCP = 201;

/**
 * @brief Incremental telnet decoder and encoder.
 *
 * feed() is stateful across calls (TCP may split frames). encode* methods produce wire bytes
 * for outbound user lines and GMCP subnegotiations.
 */
class TelnetCodec {
public:
    TelnetCodec();
    ~TelnetCodec();

    /**
     * @brief Process received bytes through the telnet state machine.
     *
     * May emit text chunks, GMCP payloads, and protocol auto-replies in wireReplies.
     * wireReplies must be sent on the socket before processing inbound app data further.
     *
     * @param bytes Raw bytes from TcpSession read.
     * @return Parsed application data and any protocol replies to send.
     */
    TelnetFeedResult feed(std::span<const std::byte> bytes);

    /**
     * @brief Encode a user command line for telnet transmission.
     *
     * Appends CRLF and escapes IAC bytes as needed.
     *
     * @param line Command text without trailing newline.
     * @return Wire-ready byte sequence.
     */
    std::vector<std::byte> encodeLine(std::string_view line);

    /**
     * @brief Encode a GMCP message body in IAC SB GMCP ... IAC SE framing.
     *
     * @param body GMCP payload (e.g. "Core.Hello {...}").
     * @return Wire-ready byte sequence with IAC doubling applied inside the payload.
     */
    std::vector<std::byte> encodeGmcp(std::string_view body);

    /**
     * @brief Whether GMCP subnegotiation has been enabled for this connection.
     * @return true after IAC DO GMCP has been negotiated.
     */
    bool gmcpEnabled() const noexcept;

    /**
     * @brief Reset parser and negotiation state (call on disconnect / reconnect).
     */
    void reset();

private:
    enum class ParsePhase {
        Plain,
        AwaitCommand,
        AwaitOption,
        AwaitSbOption,
        SbBody,
        SkipSb,
    };

    // process a single byte of telnet data
    void processByte(std::byte byte, TelnetFeedResult& result);
    // flush the accumulated text buffer (happens every feed call)
    void flushTextBuffer(TelnetFeedResult& result);
    // process a subneg byte, only flushes buffer if SE is received
    void processSubnegByte(std::uint8_t byte, TelnetFeedResult& result, bool emitPayload);

    ParsePhase phase_{ParsePhase::Plain};
    std::uint8_t pendingCommand_{0};
    bool gmcpEnabled_{false};
    bool awaitingSe_{false};
    std::string textBuffer_;
    std::string sbBuffer_;
};

} // namespace genesis::network

#endif // NETWORK_TELNET_CODEC_HPP
