#include <gui_controller.hpp>
#include <main_frame.hpp>

#include <connection_settings.hpp>
#include <dialogs/connect_dialog.hpp>
#include <dialogs/error_dialog.hpp>
#include <dialogs/settings_dialog.hpp>
#include <menu/menu_bar.hpp>
#include <mudcore/display_line.hpp>

#include <wx/dcclient.h>

namespace genesis::gui {

namespace {

constexpr unsigned kDefaultClientWidth = 80;
constexpr unsigned kDefaultClientHeight = 24;

std::pair<unsigned, unsigned> clientSizeInChars(const MainDisplay& display) {
    const wxSize clientSize = display.GetClientSize();
    if (clientSize.GetWidth() <= 0 || clientSize.GetHeight() <= 0) {
        return {kDefaultClientWidth, kDefaultClientHeight};
    }

    wxClientDC dc(const_cast<MainDisplay*>(&display));
    dc.SetFont(display.GetFont());
    const wxSize charSize = dc.GetTextExtent("M");
    if (charSize.GetWidth() <= 0 || charSize.GetHeight() <= 0) {
        return {kDefaultClientWidth, kDefaultClientHeight};
    }

    const unsigned width = static_cast<unsigned>(clientSize.GetWidth() / charSize.GetWidth());
    const unsigned height = static_cast<unsigned>(clientSize.GetHeight() / charSize.GetHeight());
    return {
        width > 0 ? width : kDefaultClientWidth,
        height > 0 ? height : kDefaultClientHeight,
    };
}

wxString phaseLabel(const mudcore::ConnectionPhase phase) {
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

GuiController::GuiController(MainClientFrame& frame)
    : frame_(frame)
    , session_(ioContext_) {}

GuiController::~GuiController() {
    shutdown();
}

void GuiController::start() {
    if (ioThread_.joinable()) {
        return;
    }

    if (ioContext_.stopped()) {
        ioContext_.restart();
    }
    workGuard_.emplace(ioContext_.get_executor());

    ioThread_ = std::thread([this]() { ioContext_.run(); });

    timer_.SetOwner(this);
    Bind(wxEVT_TIMER, &GuiController::onTimer, this, timer_.GetId());
    timer_.Start(30);

    wireFrame();
    applySettings();
    updateInputEnabled(lastPhase_);
}

void GuiController::shutdown() {
    if (shutDown_) {
        return;
    }
    shutDown_ = true;

    timer_.Stop();
    Unbind(wxEVT_TIMER, &GuiController::onTimer, this, timer_.GetId());

    session_.disconnect();

    workGuard_.reset();
    if (!ioContext_.stopped()) {
        ioContext_.stop();
    }

    if (ioThread_.joinable()) {
        ioThread_.join();
    }
}

void GuiController::connect(const std::string& host, const std::uint16_t port) {
    session_.connect(host, port);
}

void GuiController::disconnect() {
    session_.disconnect();
    frame_.inputBar().Clear();
}

void GuiController::submitCommand(const std::string_view command) {
    session_.sendCommand(command);
}

void GuiController::wireFrame() {
    if (wired_) {
        return;
    }
    wired_ = true;

    wireInputBar();
    wireMenuEvents();
    wireFooterEvents();
    wireCloseEvent();
}

void GuiController::wireInputBar() {
    frame_.inputBar().setOnSubmit([this](const std::string_view command) { submitCommand(command); });
}

void GuiController::wireMenuEvents() {
    frame_.Bind(wxEVT_MENU, &GuiController::onMenu, this, static_cast<int>(MenuId::Connect));
    frame_.Bind(wxEVT_MENU, &GuiController::onMenu, this, static_cast<int>(MenuId::Disconnect));
}

void GuiController::wireFooterEvents() {
    frame_.connectionFooter().setOnConnect([this]() { showConnectDialog(); });
    frame_.connectionFooter().setOnDisconnect([this]() { disconnect(); });
    frame_.connectionFooter().setOnSettings([this]() { showSettingsDialog(); });
}

void GuiController::wireCloseEvent() {
    frame_.Bind(wxEVT_CLOSE_WINDOW, &GuiController::onClose, this);
}

void GuiController::showConnectDialog() {
    if (lastPhase_ != mudcore::ConnectionPhase::Disconnected) {
        return;
    }

    ConnectDialog dialog(&frame_);
    if (dialog.showModal()) {
        connect(dialog.host(), dialog.port());
    }
}

void GuiController::showSettingsDialog() {
    SettingsDialog dialog(&frame_);
    if (dialog.showModal()) {
        applySettings();
    }
}

void GuiController::onMenu(wxCommandEvent& event) {
    switch (event.GetId()) {
    case static_cast<int>(MenuId::Connect):
        showConnectDialog();
        break;
    case static_cast<int>(MenuId::Disconnect):
        disconnect();
        break;
    default:
        break;
    }
}

void GuiController::onClose(wxCloseEvent& event) {
    shutdown();
    event.Skip();
}

void GuiController::onTimer(wxTimerEvent& /*event*/) {
    const mudcore::PollResult result = session_.poll();

    for (const mudcore::DisplayLine& line : result.lines) {
        routeDisplayLine(line);
    }

    if (result.connectionPhase != lastPhase_) {
        const mudcore::ConnectionPhase previousPhase = lastPhase_;
        lastPhase_ = result.connectionPhase;
        if (debugLogging_) {
            logPhaseChange(previousPhase, result.connectionPhase);
        }
        frame_.setConnectionPhase(result.connectionPhase);
        frame_.connectionFooter().setPhase(result.connectionPhase);
        frame_.menuBar().updateForPhase(result.connectionPhase);
        frame_.rightColumn().mapNotebook().selectSystemLogTab();
        updateInputEnabled(result.connectionPhase);

        if (result.connectionPhase == mudcore::ConnectionPhase::HandshakeSent) {
            const auto [width, height] = clientSizeInChars(frame_.mainDisplay());
            session_.sendClientSize(width, height);
        }

        if (result.connectionPhase == mudcore::ConnectionPhase::Ready) {
            frame_.inputBar().SetFocus();
        }

        if (result.connectionPhase == mudcore::ConnectionPhase::Disconnected) {
            lastShownError_.clear();
            clearAllDisplays();
        }
    }

    if (result.stateChanged) {
        const mudcore::RoomInfo& room = session_.gameState().room();
        frame_.vitalsBar().refresh(session_.gameState());
        frame_.magicMapPanel().setRoomDescription(room.shortDescription);
        const std::string& mapText = !room.zoom.empty() ? room.zoom : room.map;
        frame_.magicMapPanel().setMapText(mapText);
    }
}

void GuiController::routeDisplayLine(const mudcore::DisplayLine& line) {
    switch (line.sink) {
    case mudcore::OutputSink::Main:
        frame_.mainDisplay().append(line.text);
        break;
    case mudcore::OutputSink::Comms:
        frame_.comms().append(line.text);
        break;
    case mudcore::OutputSink::System:
        frame_.systemLog().append(line.text);
        if (line.text.rfind("Network error:", 0) == 0) {
            frame_.rightColumn().mapNotebook().selectSystemLogTab();
            if (line.text != lastShownError_) {
                lastShownError_ = line.text;
                ErrorDialog::show(
                    &frame_,
                    wxString::FromUTF8(line.text.data(), static_cast<int>(line.text.size())));
            }
        }
        break;
    }
}

void GuiController::updateInputEnabled(const mudcore::ConnectionPhase phase) {
    const bool canSendCommands = phase == mudcore::ConnectionPhase::Ready
        || phase == mudcore::ConnectionPhase::HandshakeSent;
    frame_.inputBar().Enable(canSendCommands);
}

void GuiController::clearAllDisplays() {
    frame_.mainDisplay().clear();
    frame_.comms().clear();
    frame_.systemLog().clear();
    frame_.magicMapPanel().clear();
    frame_.vitalsBar().clear();
}

void GuiController::applySettings() {
    const ConnectionSettings settings = loadConnectionSettings();
    debugLogging_ = settings.debugLogging;
    session_.setDebugLogging(debugLogging_);
}

void GuiController::logPhaseChange(
    const mudcore::ConnectionPhase from,
    const mudcore::ConnectionPhase to) {
    const wxString message = "Phase: " + phaseLabel(from) + " -> " + phaseLabel(to) + "\n";
    frame_.systemLog().append(message.ToUTF8().data());
}

} // namespace genesis::gui
