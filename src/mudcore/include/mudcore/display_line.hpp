/**
 * @file display_line.hpp
 * @brief GUI-facing output unit produced by Session::poll().
 *
 * Genesis MUD sends raw text without ANSI escape codes.
 */

#ifndef MUDCORE_DISPLAY_LINE_HPP
#define MUDCORE_DISPLAY_LINE_HPP

#include <string>

namespace genesis::mudcore {

/**
 * @brief Target window for a line of displayed text.
 */
enum class OutputSink {
    Main,   ///< Primary narrative output window.
    Comms,  ///< Chat / channel traffic.
    System, ///< Connection status and system messages.
};

/**
 * @brief A single line of text ready for rendering in the GUI.
 */
struct DisplayLine {
    OutputSink sink{OutputSink::Main}; ///< Which panel should render this line.
    std::string text;                  ///< Raw text content (no ANSI).
};

} // namespace genesis::mudcore

#endif // MUDCORE_DISPLAY_LINE_HPP
