#include <mudcore/outbound_pipeline.hpp>

namespace genesis::mudcore {

std::optional<std::string> OutboundPipeline::process(std::string_view input) const {
    if (input.empty()) {
        return std::nullopt;
    }
    return std::string(input);
}

} // namespace genesis::mudcore
