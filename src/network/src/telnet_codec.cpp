/**
 * @file telnet_codec.cpp
 * @brief Telnet encode/decode implementation.
 *
 * Inbound parsing is a byte-oriented finite state machine (ParsePhase). TCP may split
 * any telnet sequence across reads, so phase_ and buffers persist between feed() calls.
 *
 * State transition overview:
 *
 *   Plain --(IAC)--> AwaitCommand
 *   AwaitCommand --(IAC)--> Plain          (literal 0xFF in text)
 *   AwaitCommand --(WILL/DO/...)--> AwaitOption --(option byte)--> Plain
 *   AwaitCommand --(SB)--> AwaitSbOption --(GMCP)--> SbBody --(IAC SE)--> Plain
 *                                      \--(other)--> SkipSb --(IAC SE)--> Plain
 *
 * Text emission: accumulate in textBuffer_; flush on each \\r\\n boundary and again at
 * end of feed(). GMCP subnegotiation bodies use sbBuffer_ with IAC-doubling undone via
 * awaitingSe_ (same pattern as telnet spec: IAC in SB payload is escaped as IAC IAC).
 */

#include <network/telnet_codec.hpp>

namespace genesis::network {

namespace {

/** @brief Append the payload to the output vector, doubling IAC bytes if necessary. */
void appendDoublingIac(std::vector<std::byte>& out, std::string_view payload) {
    for (char ch : payload) {
        const auto byte = static_cast<std::uint8_t>(static_cast<unsigned char>(ch));
        out.push_back(static_cast<std::byte>(byte));
        if (byte == IAC) {
            out.push_back(static_cast<std::byte>(IAC));
        }
    }
}

/** @brief Append the GMCP frame to the output vector, doubling IAC bytes if necessary. */
void appendGmcpFrame(std::vector<std::byte>& out, std::string_view body) {
    out.push_back(static_cast<std::byte>(IAC));
    out.push_back(static_cast<std::byte>(SB));
    out.push_back(static_cast<std::byte>(GMCP));
    appendDoublingIac(out, body);
    out.push_back(static_cast<std::byte>(IAC));
    out.push_back(static_cast<std::byte>(SE));
}

/** @brief Check if the text ends with a CRLF sequence. */
bool endsWithCrlf(const std::string& text) {
    return text.size() >= 2 && text[text.size() - 2] == '\r' && text[text.size() - 1] == '\n';
}

} // namespace

TelnetCodec::TelnetCodec() = default;
TelnetCodec::~TelnetCodec() = default;

TelnetFeedResult TelnetCodec::feed(std::span<const std::byte> bytes) {
    TelnetFeedResult result;
    for (const std::byte byte : bytes) {
        processByte(byte, result);
    }
    // Emit any trailing text not yet terminated by CRLF (e.g. partial line or prompt).
    flushTextBuffer(result);
    return result;
}

void TelnetCodec::flushTextBuffer(TelnetFeedResult& result) {
    if (textBuffer_.empty()) {
        return;
    }
    result.textChunks.push_back(MudTextChunk{textBuffer_});
    textBuffer_.clear();
}

/**
 * Handle one payload byte inside IAC SB ... IAC SE (SbBody or SkipSb).
 *
 * awaitingSe_ is set when IAC is seen inside the subnegotiation; the next byte is either:
 *   - IAC  -> literal 0xFF in the payload (doubled on wire)
 *   - SE   -> end of subnegotiation; emit GmcpPayload if emitPayload, return to Plain
 *   - other -> defensive: treat as escaped IAC followed by data byte
 */
void TelnetCodec::processSubnegByte(std::uint8_t byte, TelnetFeedResult& result, const bool emitPayload) {
    // IAC recieved
    if (awaitingSe_) {
        // if recieved IAC, double it as 2x IAC is escaped byte 255
        if (byte == IAC) {
            if (emitPayload) {
                sbBuffer_.push_back(static_cast<char>(IAC));
            }
        // subnegotiation end
        } else if (byte == SE) {
            if (emitPayload) {
                // flush buffer on SE
                result.gmcpPayloads.push_back(GmcpPayload{sbBuffer_});
                sbBuffer_.clear();
            }
            phase_ = ParsePhase::Plain;
            awaitingSe_ = false;
            return;
        // other byte, add to buffer (ignore IAC x)
        } else {
            if (emitPayload) {
                sbBuffer_.push_back(static_cast<char>(IAC));
                sbBuffer_.push_back(static_cast<char>(byte));
            }
        }
        awaitingSe_ = false;
        return;
    }

    // IAC recieved, switch to awaiting SE
    if (byte == IAC) {
        awaitingSe_ = true;
        return;
    }

    // accumulate normal bytes
    if (emitPayload) {
        sbBuffer_.push_back(static_cast<char>(byte));
    }
}

/** Dispatch one received byte according to the current ParsePhase. */
void TelnetCodec::processByte(const std::byte rawByte, TelnetFeedResult& result) {
    const std::uint8_t byte = static_cast<std::uint8_t>(rawByte);

    switch (phase_) {
    case ParsePhase::Plain:
        if (byte == IAC) {
            // Leave text mode; pending text must not bleed into the telnet command.
            flushTextBuffer(result);
            phase_ = ParsePhase::AwaitCommand;
        } else {
            textBuffer_.push_back(static_cast<char>(byte));
            if (endsWithCrlf(textBuffer_)) {
                flushTextBuffer(result);
            }
        }
        break;

    case ParsePhase::AwaitCommand:
        // Stays active across feed() boundaries if a read ends on a bare IAC.
        if (byte == IAC) {
            // IAC IAC -> escaped literal 0xFF in MUD text.
            textBuffer_.push_back(static_cast<char>(IAC));
            phase_ = ParsePhase::Plain;
        } else if (byte == WILL || byte == WONT || byte == DO || byte == DONT) {
            pendingCommand_ = byte;
            phase_ = ParsePhase::AwaitOption;
        } else if (byte == SB) {
            phase_ = ParsePhase::AwaitSbOption;
        } else if (byte == SE) {
            phase_ = ParsePhase::Plain;
        } else {
            // Unhandled telnet command; discard and resume text.
            phase_ = ParsePhase::Plain;
        }
        break;

    case ParsePhase::AwaitOption:
        // Genesis sends IAC WILL GMCP; we reply IAC DO GMCP once per connection.
        if (pendingCommand_ == WILL && byte == GMCP && !gmcpEnabled_) {
            result.wireReplies.push_back(static_cast<std::byte>(IAC));
            result.wireReplies.push_back(static_cast<std::byte>(DO));
            result.wireReplies.push_back(static_cast<std::byte>(GMCP));
            gmcpEnabled_ = true;
            result.negotiatedNow = true;
        }
        phase_ = ParsePhase::Plain;
        break;

    case ParsePhase::AwaitSbOption:
        if (byte == GMCP) {
            sbBuffer_.clear();
            awaitingSe_ = false;
            phase_ = ParsePhase::SbBody;
        } else {
            // Non-GMCP subnegotiation: scan to IAC SE without emitting.
            awaitingSe_ = false;
            phase_ = ParsePhase::SkipSb;
        }
        break;

    case ParsePhase::SbBody:
        processSubnegByte(byte, result, true);
        break;

    case ParsePhase::SkipSb:
        processSubnegByte(byte, result, false);
        break;
    }
}

std::vector<std::byte> TelnetCodec::encodeLine(const std::string_view line) {
    std::vector<std::byte> encoded;
    encoded.reserve(line.size() + 2);
    appendDoublingIac(encoded, line);
    encoded.push_back(static_cast<std::byte>('\r'));
    encoded.push_back(static_cast<std::byte>('\n'));
    return encoded;
}

std::vector<std::byte> TelnetCodec::encodeGmcp(const std::string_view body) {
    std::vector<std::byte> encoded;
    encoded.reserve(body.size() + 6);
    appendGmcpFrame(encoded, body);
    return encoded;
}

bool TelnetCodec::gmcpEnabled() const noexcept {
    return gmcpEnabled_;
}

/** @brief Reset the codec state to initial values. */
void TelnetCodec::reset() {
    phase_ = ParsePhase::Plain;
    pendingCommand_ = 0;
    gmcpEnabled_ = false;
    awaitingSe_ = false;
    textBuffer_.clear();
    sbBuffer_.clear();
}

} // namespace genesis::network
