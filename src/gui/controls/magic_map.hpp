#ifndef GENESIS_GUI_CONTROLS_MAGIC_MAP_HPP
#define GENESIS_GUI_CONTROLS_MAGIC_MAP_HPP

#include <wx/wx.h>

#include <optional>
#include <string_view>

namespace genesis::gui {

class MagicMap : public wxTextCtrl {
public:
    explicit MagicMap(wxWindow* parent);
    ~MagicMap() override = default;

    MagicMap(const MagicMap&) = delete;
    MagicMap& operator=(const MagicMap&) = delete;

    /**
     * @brief Replace map contents; optionally color the character at (markerX, markerY) red.
     *
     * Coordinates match Room.Info: y is line index, x is column index within that line.
     * After update, the view scrolls to the top so large maps remain usable.
     */
    void setMapText(
        std::string_view text,
        std::optional<int> markerX = std::nullopt,
        std::optional<int> markerY = std::nullopt);
};

} // namespace genesis::gui

#endif // GENESIS_GUI_CONTROLS_MAGIC_MAP_HPP
