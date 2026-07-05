#include <panels/system_log.hpp>

namespace genesis::gui {

SystemLogPanel::SystemLogPanel(wxWindow* parent)
    : wxPanel(parent) {
    log_ = new wxTextCtrl(
        this,
        wxID_ANY,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        wxTE_MULTILINE | wxTE_READONLY);

    auto* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(log_, 1, wxEXPAND | wxALL, 4);
    SetSizer(sizer);
}

void SystemLogPanel::appendLine(const std::string_view text) {
    log_->AppendText(wxString::FromUTF8(text.data(), static_cast<int>(text.size())));
    log_->AppendText("\n");
}

} // namespace genesis::gui
