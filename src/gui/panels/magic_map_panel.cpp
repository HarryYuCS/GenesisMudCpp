#include <panels/magic_map_panel.hpp>

namespace genesis::gui {

MagicMapPanel::MagicMapPanel(wxWindow* parent)
    : wxPanel(parent) {
    map_ = new MagicMap(this);

    auto* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(map_, 1, wxEXPAND | wxALL, 4);
    SetSizer(sizer);
}

void MagicMapPanel::setMapText(const std::string_view text) {
    map_->setMapText(text);
}

MagicMap& MagicMapPanel::map() {
    return *map_;
}

} // namespace genesis::gui
