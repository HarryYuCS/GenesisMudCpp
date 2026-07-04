/**
 * @file string_utils.hpp
 * @brief Small string helpers for mudcore (header-only).
 */

#ifndef MUDCORE_STRING_UTILS_HPP
#define MUDCORE_STRING_UTILS_HPP

#include <algorithm>
#include <cctype>
#include <string_view>

namespace genesis::mudcore {

inline bool equalsIgnoreCase(const std::string_view left, const std::string_view right) {
    return left.size() == right.size()
        && std::equal(left.begin(), left.end(), right.begin(), right.end(), [](const char a, const char b) {
               return std::tolower(static_cast<unsigned char>(a))
                   == std::tolower(static_cast<unsigned char>(b));
           });
}

inline bool startsWithIgnoreCase(const std::string_view text, const std::string_view prefix) {
    return text.size() >= prefix.size() && equalsIgnoreCase(text.substr(0, prefix.size()), prefix);
}

} // namespace genesis::mudcore

#endif // MUDCORE_STRING_UTILS_HPP
