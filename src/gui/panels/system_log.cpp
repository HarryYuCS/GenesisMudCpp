#include <panels/system_log.hpp>

#include <wx/datetime.h>

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

void SystemLogPanel::append(const std::string_view text) {
    std::string_view body = text;
    while (!body.empty() && (body.back() == '\n' || body.back() == '\r')) {
        body.remove_suffix(1);
    }
    if (body.empty()) {
        return;
    }

    const wxString stamp = wxDateTime::Now().Format("%H:%M:%S");
    const wxString message = wxString::FromUTF8(body.data(), static_cast<int>(body.size()));
    log_->AppendText(wxString::Format("[%s] %s\n", stamp, message));
    log_->ShowPosition(log_->GetLastPosition());
}

void SystemLogPanel::clear() {
    log_->Clear();
}

} // namespace genesis::gui
