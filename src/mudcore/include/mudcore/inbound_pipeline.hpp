/**
 * @file inbound_pipeline.hpp
 * @brief Transforms inbound server data into DisplayLines and updates GameState.
 */

#ifndef MUDCORE_INBOUND_PIPELINE_HPP
#define MUDCORE_INBOUND_PIPELINE_HPP

#include <mudcore/display_line.hpp>
#include <mudcore/game_state.hpp>
#include <mudcore/gmcp_parser.hpp>

#include <string_view>
#include <vector>

namespace genesis::mudcore {

/**
 * @brief Pure processing pipeline for server-to-client data.
 *
 * Returns DisplayLines for the GUI and may update GameState from GMCP.
 * Does not enqueue Events or interact with the EventBus — Session owns that.
 */
class InboundPipeline {
public:
    /**
     * @brief Wrap raw MUD text as a main-window DisplayLine.
     *
     * Trigger matching and additional routing will be added here later.
     *
     * @param text Player-visible text from telnet (no IAC bytes).
     * @return One or more lines to append to the GUI.
     */
    std::vector<DisplayLine> processMudText(std::string_view text) const;

    /**
     * @brief Process a parsed GMCP message: update state and optionally emit display lines.
     *
     * Comm.* packages are routed to OutputSink::Comms; other packages may only update GameState.
     *
     * @param message Parsed GMCP message.
     * @param gameState GameState to update (main thread only).
     * @return Display lines for chat or system output; may be empty.
     */
    std::vector<DisplayLine> processGmcp(const GmcpMessage& message, GameState& gameState) const;
};

} // namespace genesis::mudcore

#endif // MUDCORE_INBOUND_PIPELINE_HPP
