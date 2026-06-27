/**
 * @file outbound_pipeline.hpp
 * @brief Transforms user input before it is sent to the MUD.
 */

#ifndef MUDCORE_OUTBOUND_PIPELINE_HPP
#define MUDCORE_OUTBOUND_PIPELINE_HPP

#include <optional>
#include <string>
#include <string_view>

namespace genesis::mudcore {

/**
 * @brief Client-to-server input processing (alias expansion, etc.).
 *
 * Session calls process() on the main thread and posts the result to the io thread for sending.
 */
class OutboundPipeline {
public:
    /**
     * @brief Process a user-entered command line.
     *
     * @param input Raw text from the input bar.
     * @return Expanded command to send, or std::nullopt if an alias fully consumed the input.
     */
    std::optional<std::string> process(std::string_view input) const;
};

} // namespace genesis::mudcore

#endif // MUDCORE_OUTBOUND_PIPELINE_HPP
