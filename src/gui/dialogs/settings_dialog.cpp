#include <dialogs/settings_dialog.hpp>

#include <connection_settings.hpp>

namespace genesis::gui {

SettingsDialog::SettingsDialog(wxWindow* parent)
    : wxDialog(parent, wxID_ANY, "Settings", wxDefaultPosition, wxSize(360, 200)) {
    const ConnectionSettings settings = loadConnectionSettings();

    auto* hostLabel = new wxStaticText(this, wxID_ANY, "Default host:");
    hostField_ = new wxTextCtrl(
        this,
        wxID_ANY,
        wxString::FromUTF8(settings.host.data(), static_cast<int>(settings.host.size())));

    auto* portLabel = new wxStaticText(this, wxID_ANY, "Default port:");
    portField_ = new wxTextCtrl(this, wxID_ANY, wxString::Format("%u", settings.port));

    debugLoggingCheck_ = new wxCheckBox(this, wxID_ANY, "Debug logging (GMCP + phase changes to System Log)");
    debugLoggingCheck_->SetValue(settings.debugLogging);

    auto* grid = new wxFlexGridSizer(2, 8, 8);
    grid->Add(hostLabel, 0, wxALIGN_CENTER_VERTICAL);
    grid->Add(hostField_, 1, wxEXPAND);
    grid->Add(portLabel, 0, wxALIGN_CENTER_VERTICAL);
    grid->Add(portField_, 1, wxEXPAND);
    grid->AddGrowableCol(1);

    auto* buttons = CreateStdDialogButtonSizer(wxOK | wxCANCEL);
    auto* root = new wxBoxSizer(wxVERTICAL);
    root->Add(grid, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 12);
    root->Add(debugLoggingCheck_, 0, wxEXPAND | wxLEFT | wxRIGHT, 12);
    root->Add(buttons, 0, wxEXPAND | wxALL, 12);
    SetSizer(root);

    Bind(wxEVT_BUTTON, &SettingsDialog::onSave, this, wxID_OK);
}

bool SettingsDialog::showModal() {
    return ShowModal() == wxID_OK;
}

void SettingsDialog::onSave(wxCommandEvent& event) {
    const wxString hostValue = hostField_->GetValue().Trim();
    const wxString portValue = portField_->GetValue().Trim();

    if (hostValue.IsEmpty()) {
        wxMessageBox("Host is required.", "Settings", wxOK | wxICON_WARNING, this);
        return;
    }

    long portNumber = 0;
    if (!portValue.ToLong(&portNumber) || portNumber < 1 || portNumber > 65535) {
        wxMessageBox("Port must be a number between 1 and 65535.", "Settings", wxOK | wxICON_WARNING, this);
        return;
    }

    ConnectionSettings settings;
    settings.host = hostValue.ToUTF8().data();
    settings.port = static_cast<std::uint16_t>(portNumber);
    settings.debugLogging = debugLoggingCheck_->GetValue();
    saveConnectionSettings(settings);
    event.Skip();
}

} // namespace genesis::gui
