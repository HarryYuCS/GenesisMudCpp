#include <main_frame.hpp>

#include <menu/menu_bar.hpp>

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

MainClientFrame::MainClientFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
    : wxFrame(nullptr, wxID_ANY, title, pos, size)
    , baseTitle_(title) {
    auto* rootSizer = new wxBoxSizer(wxVERTICAL);

    auto* topSizer = new wxFlexGridSizer(1, 2, 4, 4);
    topSizer->AddGrowableCol(0);
    topSizer->AddGrowableCol(1);
    topSizer->AddGrowableRow(0);

    mainDisplay_ = new MainDisplay(this);
    rightColumn_ = new RightColumnPanel(this);

    topSizer->Add(mainDisplay_, 1, wxEXPAND | wxALL, 4);
    topSizer->Add(rightColumn_, 1, wxEXPAND | wxALL, 4);
    rootSizer->Add(topSizer, 1, wxEXPAND);

    auto* bottomSizer = new wxBoxSizer(wxVERTICAL);
    inputBar_ = new InputBar(this);
    vitalsBar_ = new VitalsBarPanel(this);
    connectionFooter_ = new ConnectionFooterPanel(this);

    bottomSizer->Add(inputBar_, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 4);
    bottomSizer->Add(vitalsBar_, 0, wxEXPAND | wxLEFT | wxRIGHT, 4);
    bottomSizer->Add(connectionFooter_, 0, wxEXPAND | wxALL, 4);
    rootSizer->Add(bottomSizer, 0, wxEXPAND);

    SetSizer(rootSizer);
    menuBar_ = new MenuBar();
    SetMenuBar(menuBar_);
    setConnectionPhase(mudcore::ConnectionPhase::Disconnected);
    connectionFooter_->setPhase(mudcore::ConnectionPhase::Disconnected);
    menuBar_->updateForPhase(mudcore::ConnectionPhase::Disconnected);
}

MainDisplay& MainClientFrame::mainDisplay() {
    return *mainDisplay_;
}

RightColumnPanel& MainClientFrame::rightColumn() {
    return *rightColumn_;
}

CommsPanel& MainClientFrame::comms() {
    return rightColumn_->comms();
}

MagicMapPanel& MainClientFrame::magicMapPanel() {
    return rightColumn_->mapNotebook().magicMapPanel();
}

InputBar& MainClientFrame::inputBar() {
    return *inputBar_;
}

VitalsBarPanel& MainClientFrame::vitalsBar() {
    return *vitalsBar_;
}

SystemLogPanel& MainClientFrame::systemLog() {
    return rightColumn_->mapNotebook().systemLogPanel();
}

ConnectionFooterPanel& MainClientFrame::connectionFooter() {
    return *connectionFooter_;
}

MenuBar& MainClientFrame::menuBar() {
    return *menuBar_;
}

void MainClientFrame::setConnectionPhase(const mudcore::ConnectionPhase phase) {
    if (phase == lastPhase_) {
        return;
    }
    lastPhase_ = phase;
    SetTitle(baseTitle_ + " — " + phaseLabel(phase));
}

} // namespace genesis::gui
