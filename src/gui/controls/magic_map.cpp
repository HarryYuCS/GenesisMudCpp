#include <controls/magic_map.hpp>

namespace genesis::gui {

namespace {

constexpr int kMagicMapFontPointSize = 7;

/** @brief Character offset of (x, y) in newline-separated mapText, or -1 if out of range. */
long markerCharIndex(std::string_view mapText, int x, int y) {
    if (x < 0 || y < 0) {
        return -1;
    }

    int line = 0;
    std::size_t lineStart = 0;
    for (std::size_t i = 0; i <= mapText.size(); ++i) {
        const bool atEnd = i == mapText.size();
        const bool atNewline = !atEnd && mapText[i] == '\n';
        if (!atEnd && !atNewline) {
            continue;
        }

        if (line == y) {
            const std::size_t lineLen = i - lineStart;
            if (static_cast<std::size_t>(x) >= lineLen) {
                return -1;
            }
            return static_cast<long>(lineStart + static_cast<std::size_t>(x));
        }

        if (atEnd) {
            break;
        }
        lineStart = i + 1;
        ++line;
    }
    return -1;
}

} // namespace

MagicMap::MagicMap(wxWindow* parent)
    : wxTextCtrl(
          parent,
          wxID_ANY,
          wxEmptyString,
          wxDefaultPosition,
          wxDefaultSize,
          wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH2 | wxTE_DONTWRAP) {
    SetFont(wxFont(
        kMagicMapFontPointSize,
        wxFONTFAMILY_TELETYPE,
        wxFONTSTYLE_NORMAL,
        wxFONTWEIGHT_NORMAL));
    SetDefaultStyle(wxTextAttr(GetForegroundColour()));
}

void MagicMap::setMapText(
    const std::string_view text,
    const std::optional<int> markerX,
    const std::optional<int> markerY) {
    Freeze();

    const wxColour normalColour = GetForegroundColour();
    SetDefaultStyle(wxTextAttr(normalColour));
    ChangeValue(wxString::FromUTF8(text.data(), static_cast<int>(text.size())));

    if (markerX.has_value() && markerY.has_value()) {
        const long pos = markerCharIndex(text, *markerX, *markerY);
        if (pos >= 0) {
            wxTextAttr markerAttr(*wxRED);
            markerAttr.SetFont(GetFont());
            SetStyle(pos, pos + 1, markerAttr);
        }
    }

    // Keep the map pinned to the top-left on refresh so scroll position does not
    // jump and hide part of the grid after each Room.Info / Room.Map update.
    SetInsertionPoint(0);
    ShowPosition(0);

    Thaw();
}

} // namespace genesis::gui
