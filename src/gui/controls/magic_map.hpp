#ifndef GENESIS_GUI_CONTROLS_MAGIC_MAP_HPP
#define GENESIS_GUI_CONTROLS_MAGIC_MAP_HPP

#include <wx/wx.h>

#include <string_view>

namespace genesis::gui {

class MagicMap : public wxTextCtrl {
public:
    explicit MagicMap(wxWindow* parent);
    ~MagicMap() override = default;

    MagicMap(const MagicMap&) = delete;
    MagicMap& operator=(const MagicMap&) = delete;

    void setMapText(std::string_view text);
};

} // namespace genesis::gui

#endif // GENESIS_GUI_CONTROLS_MAGIC_MAP_HPP
