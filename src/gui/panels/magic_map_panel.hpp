#ifndef GENESIS_GUI_PANELS_MAGIC_MAP_PANEL_HPP
#define GENESIS_GUI_PANELS_MAGIC_MAP_PANEL_HPP

#include <controls/magic_map.hpp>

#include <wx/wx.h>

#include <optional>
#include <string_view>

namespace genesis::gui {

class MagicMapPanel : public wxPanel {
public:
    explicit MagicMapPanel(wxWindow* parent);
    ~MagicMapPanel() override = default;

    MagicMapPanel(const MagicMapPanel&) = delete;
    MagicMapPanel& operator=(const MagicMapPanel&) = delete;

    void setRoomDescription(std::string_view text);
    void setMapText(
        std::string_view text,
        std::optional<int> markerX = std::nullopt,
        std::optional<int> markerY = std::nullopt);
    void clear();
    MagicMap& map();

private:
    wxStaticText* roomLabel_{nullptr};
    MagicMap* map_{nullptr};
};

} // namespace genesis::gui

#endif // GENESIS_GUI_PANELS_MAGIC_MAP_PANEL_HPP
