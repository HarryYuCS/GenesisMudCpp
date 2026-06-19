#ifndef MUDCORE_INPUT_PIPELINE_HPP
#define MUDCORE_INPUT_PIPELINE_HPP

#include <mudcore/event.hpp>

namespace genesis::mudcore {

/**
 * @brief The InputPipeline class is responsible for processing input from the user and converting it into a ClientEvent.
 *
 * This class is responsible for handling the ClientEvent creation from user input, including handling aliases.
 */
class InputPipeline {
public:
    InputPipeline();
    ~InputPipeline();

    ClientEvent processCommand(const std::string& command);
    ClientEvent processConnectRequest();
    ClientEvent processDisconnectRequest();
};

} // namespace genesis::mudcore

#endif // MUDCORE_INPUT_PIPELINE_HPP