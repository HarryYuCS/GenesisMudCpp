/**
 * @file inbound_pipeline.hpp
 * @brief Transforms inbound server data into DisplayLines and updates GameState.
 */

#ifndef MUDCORE_INBOUND_PIPELINE_HPP
#define MUDCORE_INBOUND_PIPELINE_HPP

#include <mudcore/display_line.hpp>
#include <mudcore/game_state.hpp>
#include <mudcore/gmcp_parser.hpp>

#include <optional>
#include <string_view>

namespace genesis::mudcore {

/**
 * @brief Result of processing one raw GMCP body through the inbound pipeline.
 *
 * stateChanged and playerLoggedIn are orthogonal effects (a Char.Login both changes
 * state and logs the player in) and drive different consumers (UI refresh vs. connection
 * lifecycle), so they are separate bools rather than a one-of-N enum. If a third
 * lifecycle-relevant effect is added, switch to a flags enum instead of growing bools.
 */
struct GmcpInboundResult {
    std::optional<DisplayLine> line; ///< Comms display line, if applicable.
    bool stateChanged{false};        ///< true if GameState was updated (GUI should refresh).
    bool playerLoggedIn{false};      ///< true if this message was Char.Login and state is now logged in.
};

/**
 * @brief Pure processing pipeline for server-to-client data.
 *
 * Returns at most one DisplayLine per call; Session aggregates across events in poll().
 * Parses GMCP and may update GameState. Does not enqueue Events or interact with the EventBus.
 */
class InboundPipeline {
public:
    /**
     * @brief Wrap raw MUD text as a main-window DisplayLine.
     *
     * @param text Player-visible text from telnet (no IAC bytes).
     * @return A line for the main window, or std::nullopt for empty input.
     */
    std::optional<DisplayLine> processMudText(std::string_view text) const;

    /**
     * @brief Parse and process a raw GMCP body from telnet subnegotiation.
     *
     * Comm.* packages are routed to OutputSink::Comms; other packages update GameState.
     *
     * @param rawBody Full GMCP body (e.g. "Char.Vitals {...}").
     * @param gameState GameState to update (main thread only).
     * @return GmcpInboundResult containing the optional display line and the boolean flags for stateChanged and playerLoggedIn.
     */
    GmcpInboundResult processGmcp(std::string_view rawBody, GameState& gameState) const;

private:
    GmcpInboundResult processParsedGmcp(const GmcpMessage& message, GameState& gameState) const;

    GmcpParser gmcpParser_;
};

} // namespace genesis::mudcore

#endif // MUDCORE_INBOUND_PIPELINE_HPP
