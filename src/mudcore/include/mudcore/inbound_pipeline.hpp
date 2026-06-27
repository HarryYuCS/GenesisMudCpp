#ifndef MUDCORE_INBOUND_PIPELINE_HPP
#define MUDCORE_INBOUND_PIPELINE_HPP

#include <mudcore/display_line.hpp>
#include <mudcore/game_state.hpp>
#include <mudcore/gmcp_parser.hpp>

#include <string_view>
#include <vector>

namespace genesis::mudcore {

/**
 * @brief Transforms inbound server data into DisplayLines and updates GameState from GMCP.
 *
 * Does not create Events or touch the EventBus — Session owns queueing.
 */
class InboundPipeline {
public:
    std::vector<DisplayLine> processMudText(std::string_view text) const;
    std::vector<DisplayLine> processGmcp(const GmcpMessage& message, GameState& gameState) const;
};

} // namespace genesis::mudcore

#endif // MUDCORE_INBOUND_PIPELINE_HPP
