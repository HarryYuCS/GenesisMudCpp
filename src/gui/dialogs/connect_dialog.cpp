#include <dialogs/connect_dialog.hpp>

#include <connection_settings.hpp>

namespace genesis::gui {

ConnectDialog::ConnectDialog(wxWindow* parent)
    : wxDialog(parent, wxID_ANY, "Connect to MUD", wxDefaultPosition, wxSize(360, 160)) {
    const ConnectionSettings defaults = loadConnectionSettings();

    auto* hostLabel = new wxStaticText(this, wxID_ANY, "Host:");
    hostField_ = new wxTextCtrl(
        this,
        wxID_ANY,
        wxString::FromUTF8(defaults.host.data(), static_cast<int>(defaults.host.size())));

    auto* portLabel = new wxStaticText(this, wxID_ANY, "Port:");
    portField_ = new wxTextCtrl(this, wxID_ANY, wxString::Format("%u", defaults.port));

    auto* grid = new wxFlexGridSizer(2, 8, 8);
    grid->Add(hostLabel, 0, wxALIGN_CENTER_VERTICAL);
    grid->Add(hostField_, 1, wxEXPAND);
    grid->Add(portLabel, 0, wxALIGN_CENTER_VERTICAL);
    grid->Add(portField_, 1, wxEXPAND);
    grid->AddGrowableCol(1);

    auto* buttons = CreateStdDialogButtonSizer(wxOK | wxCANCEL);
    auto* root = new wxBoxSizer(wxVERTICAL);
    root->Add(grid, 1, wxEXPAND | wxALL, 12);
    root->Add(buttons, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 12);
    SetSizer(root);

    Bind(wxEVT_BUTTON, &ConnectDialog::onOk, this, wxID_OK);
}

bool ConnectDialog::showModal() {
    return ShowModal() == wxID_OK;
}

const std::string& ConnectDialog::host() const {
    return host_;
}

std::uint16_t ConnectDialog::port() const {
    return port_;
}

void ConnectDialog::onOk(wxCommandEvent& event) {
    const wxString hostValue = hostField_->GetValue().Trim();
    const wxString portValue = portField_->GetValue().Trim();

    if (hostValue.IsEmpty()) {
        wxMessageBox("Host is required.", "Connect", wxOK | wxICON_WARNING, this);
        return;
    }

    long portNumber = 0;
    if (!portValue.ToLong(&portNumber) || portNumber < 1 || portNumber > 65535) {
        wxMessageBox("Port must be a number between 1 and 65535.", "Connect", wxOK | wxICON_WARNING, this);
        return;
    }

    host_ = hostValue.ToUTF8().data();
    port_ = static_cast<std::uint16_t>(portNumber);
    ConnectionSettings settings = loadConnectionSettings();
    settings.host = host_;
    settings.port = port_;
    saveConnectionSettings(settings);
    event.Skip();
}

} // namespace genesis::gui
