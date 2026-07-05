#include <panels/comms.hpp>

namespace genesis::gui {

CommsPanel::CommsPanel(wxWindow* parent)
    : wxPanel(parent) {
    auto* sizer = new wxBoxSizer(wxVERTICAL);

    auto* label = new wxStaticText(this, wxID_ANY, "COMMUNICATION");
    log_ = new wxTextCtrl(
        this,
        wxID_ANY,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        wxTE_MULTILINE | wxTE_READONLY);

    sizer->Add(label, 0, wxLEFT | wxRIGHT | wxTOP, 4);
    sizer->Add(log_, 1, wxEXPAND | wxALL, 4);
    SetSizer(sizer);
}

void CommsPanel::appendLine(const std::string_view text) {
    log_->AppendText(wxString::FromUTF8(text.data(), static_cast<int>(text.size())));
    log_->AppendText("\n");
}

} // namespace genesis::gui
