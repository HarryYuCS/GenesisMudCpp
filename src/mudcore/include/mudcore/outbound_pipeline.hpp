#ifndef MUDCORE_OUTBOUND_PIPELINE_HPP
#define MUDCORE_OUTBOUND_PIPELINE_HPP

#include <optional>
#include <string>
#include <string_view>

namespace genesis::mudcore {

/**
 * @brief Transforms user input before send (alias expansion later).
 *
 * Returns nullopt if the line was fully consumed by an alias. Session posts the result to the io thread.
 */
class OutboundPipeline {
public:
    std::optional<std::string> process(std::string_view input) const;
};

} // namespace genesis::mudcore

#endif // MUDCORE_OUTBOUND_PIPELINE_HPP
