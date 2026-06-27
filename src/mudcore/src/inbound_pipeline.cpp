#include <mudcore/inbound_pipeline.hpp>

namespace genesis::mudcore {

std::vector<DisplayLine> InboundPipeline::processMudText(std::string_view text) const {
    return {DisplayLine{OutputSink::Main, std::string(text)}};
}

std::vector<DisplayLine> InboundPipeline::processGmcp(const GmcpMessage& message, GameState& gameState) const {
    gameState.applyGmcp(message);

    if (message.package.rfind("Comm.", 0) == 0) {
        return {DisplayLine{OutputSink::Comms, message.jsonBody}};
    }

    return {};
}

} // namespace genesis::mudcore
