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

} // namespace

ConnectionFooterPanel::ConnectionFooterPanel(wxWindow* parent)
    : wxPanel(parent) {
    phaseLabel_ = new wxStaticText(this, wxID_ANY, phaseLabel(mudcore::ConnectionPhase::Disconnected));

    auto* connectButton = new wxButton(this, wxID_ANY, "Connect");
    auto* settingsButton = new wxButton(this, wxID_ANY, "Settings");

    auto* sizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->AddStretchSpacer(1);
    sizer->Add(phaseLabel_, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);
    sizer->Add(connectButton, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);
    sizer->Add(settingsButton, 0, wxALIGN_CENTER_VERTICAL);
    SetSizer(sizer);
}

void ConnectionFooterPanel::setPhase(const mudcore::ConnectionPhase phase) {
    phaseLabel_->SetLabel(phaseLabel(phase));
}

} // namespace genesis::gui
