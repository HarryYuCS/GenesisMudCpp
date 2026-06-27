#ifndef MUDCORE_DISPLAY_LINE_HPP
#define MUDCORE_DISPLAY_LINE_HPP

#include <string>

namespace genesis::mudcore {

enum class OutputSink {
    Main,
    Comms,
    System,
};

/** GUI-facing line of text (Genesis sends raw text, no ANSI). */
struct DisplayLine {
    OutputSink sink{OutputSink::Main};
    std::string text;
};

} // namespace genesis::mudcore

#endif // MUDCORE_DISPLAY_LINE_HPP
