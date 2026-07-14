#include <panels/connection_footer.hpp>

namespace genesis::gui {

namespace {

wxString phaseLabel(mudcore::ConnectionPhase phase) {
    switch (phase) {
    case mudcore::ConnectionPhase::Disconnected:
        return "Disconnected";
    case mudcore::ConnectionPhase::Connecting:
        return "Connecting";
    case mudcore::ConnectionPhase::Connected:
        return "Connected";
    case mudcore::ConnectionPhase::GmcpEnabled:
        return "GMCP Enabled";
    case mudcore::ConnectionPhase::HandshakeSent:
        return "Handshake Sent";
    case mudcore::ConnectionPhase::Ready:
        return "Ready";
    }
    return "Unknown";
}

bool isDisconnected(mudcore::ConnectionPhase phase) {
    return phase == mudcore::ConnectionPhase::Disconnected;
}

} // namespace

ConnectionFooterPanel::ConnectionFooterPanel(wxWindow* parent)
    : wxPanel(parent) {
    phaseLabel_ = new wxStaticText(this, wxID_ANY, phaseLabel(mudcore::ConnectionPhase::Disconnected));
    connectButton_ = new wxButton(this, wxID_ANY, "Connect");
    auto* settingsButton = new wxButton(this, wxID_ANY, "Settings");

    connectButton_->Bind(wxEVT_BUTTON, &ConnectionFooterPanel::onConnectButton, this);
    settingsButton->Bind(wxEVT_BUTTON, &ConnectionFooterPanel::onSettingsButton, this);

    auto* sizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->AddStretchSpacer(1);
    sizer->Add(phaseLabel_, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);
    sizer->Add(connectButton_, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);
    sizer->Add(settingsButton, 0, wxALIGN_CENTER_VERTICAL);
    SetSizer(sizer);
}

void ConnectionFooterPanel::setPhase(const mudcore::ConnectionPhase phase) {
    phase_ = phase;
    phaseLabel_->SetLabel(phaseLabel(phase));

    if (isDisconnected(phase)) {
        connectButton_->SetLabel("Connect");
        connectButton_->Enable(true);
        return;
    }

    if (phase == mudcore::ConnectionPhase::Connecting) {
        connectButton_->SetLabel("Connecting...");
        connectButton_->Enable(false);
        return;
    }

    connectButton_->SetLabel("Disconnect");
    connectButton_->Enable(true);
}

void ConnectionFooterPanel::setOnConnect(std::function<void()> callback) {
    onConnect_ = std::move(callback);
}

void ConnectionFooterPanel::setOnDisconnect(std::function<void()> callback) {
    onDisconnect_ = std::move(callback);
}

void ConnectionFooterPanel::setOnSettings(std::function<void()> callback) {
    onSettings_ = std::move(callback);
}

void ConnectionFooterPanel::onConnectButton(wxCommandEvent& /*event*/) {
    if (isDisconnected(phase_)) {
        if (onConnect_) {
            onConnect_();
        }
        return;
    }

    if (onDisconnect_) {
        onDisconnect_();
    }
}

void ConnectionFooterPanel::onSettingsButton(wxCommandEvent& /*event*/) {
    if (onSettings_) {
        onSettings_();
    }
}

} // namespace genesis::gui
