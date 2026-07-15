/**
 * @file map_overlay.hpp
 * @brief Pure helpers for overlaying markers onto Room.Map ASCII grids.
 */

#ifndef MUDCORE_MAP_OVERLAY_HPP
#define MUDCORE_MAP_OVERLAY_HPP

#include <string>
#include <string_view>

namespace genesis::mudcore {

/**
 * @brief Replace the character at (x, y) in mapText with 'X'.
 *
 * Coordinates match Genesis Room.Info: y is the 0-based line index (top to bottom),
 * x is the 0-based character index within that line (left to right). Magic maps are ASCII.
 *
 * @param mapText Full map graphics string (lines separated by '\\n').
 * @param x Column index.
 * @param y Line index.
 * @return Map with marker applied, or the original string if (x, y) is out of range.
 */
std::string overlayPlayerMarker(std::string_view mapText, int x, int y);

} // namespace genesis::mudcore

#endif // MUDCORE_MAP_OVERLAY_HPP
