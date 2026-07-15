#include <panels/right_column.hpp>

namespace genesis::gui {

RightColumnPanel::RightColumnPanel(wxWindow* parent)
    : wxPanel(parent) {
    mapNotebook_ = new MapNotebookPanel(this);
    comms_ = new CommsPanel(this);

    auto* sizer = new wxBoxSizer(wxVERTICAL);
    // Give MAGIC MAP / SYSTEM most of the column; keep COMMS as a smaller strip.
    sizer->Add(mapNotebook_, 3, wxEXPAND | wxALL, 4);
    sizer->Add(comms_, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 4);
    SetSizer(sizer);
}

MapNotebookPanel& RightColumnPanel::mapNotebook() {
    return *mapNotebook_;
}

CommsPanel& RightColumnPanel::comms() {
    return *comms_;
}

} // namespace genesis::gui
