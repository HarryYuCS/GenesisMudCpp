#include <panels/magic_map_panel.hpp>

namespace genesis::gui {

MagicMapPanel::MagicMapPanel(wxWindow* parent)
    : wxPanel(parent) {
    roomLabel_ = new wxStaticText(this, wxID_ANY, "");
    map_ = new MagicMap(this);

    auto* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(roomLabel_, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 4);
    sizer->Add(map_, 1, wxEXPAND | wxALL, 4);
    SetSizer(sizer);
}

void MagicMapPanel::setRoomDescription(const std::string_view text) {
    roomLabel_->SetLabel(
        text.empty() ? wxString{} : wxString::FromUTF8(text.data(), static_cast<int>(text.size())));
}

void MagicMapPanel::setMapText(const std::string_view text) {
    map_->setMapText(text);
}

void MagicMapPanel::clear() {
    setRoomDescription("");
    setMapText("");
}

MagicMap& MagicMapPanel::map() {
    return *map_;
}

} // namespace genesis::gui
