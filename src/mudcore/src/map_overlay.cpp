#include <mudcore/map_overlay.hpp>

#include <cstddef>
#include <vector>

namespace genesis::mudcore {

std::string overlayPlayerMarker(std::string_view mapText, int x, int y) {
    if (x < 0 || y < 0) {
        return std::string(mapText);
    }

    std::vector<std::string> lines;
    std::size_t start = 0;
    while (start <= mapText.size()) {
        const std::size_t end = mapText.find('\n', start);
        if (end == std::string_view::npos) {
            lines.emplace_back(mapText.substr(start));
            break;
        }
        lines.emplace_back(mapText.substr(start, end - start));
        start = end + 1;
    }

    const auto lineIndex = static_cast<std::size_t>(y);
    const auto colIndex = static_cast<std::size_t>(x);
    if (lineIndex >= lines.size() || colIndex >= lines[lineIndex].size()) {
        return std::string(mapText);
    }

    lines[lineIndex][colIndex] = 'X';

    std::string result;
    for (std::size_t i = 0; i < lines.size(); ++i) {
        if (i > 0) {
            result.push_back('\n');
        }
        result += lines[i];
    }
    return result;
}

} // namespace genesis::mudcore
