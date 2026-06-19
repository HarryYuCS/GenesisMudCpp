#ifndef MUDCORE_OUTPUT_PIPELINE_HPP
#define MUDCORE_OUTPUT_PIPELINE_HPP

#include <mudcore/gmcp_parser.hpp>
#include <mudcore/event.hpp>

namespace genesis::mudcore {

/**
 * @brief The OutputPipeline class is responsible for processing output from the server and converting it into a ServerEvent.
 *
 * This class is responsible for handling the ServerEvent creation from server output, including 
 * handling triggers.
 */
class OutputPipeline {
public:
    OutputPipeline();
    ~OutputPipeline();

    ServerEvent processServerOutput(const std::string& output);

private:
    GMCPParser gmcpParser;
};

} // namespace genesis::mudcore

#endif // MUDCORE_OUTPUT_PIPELINE_HPP