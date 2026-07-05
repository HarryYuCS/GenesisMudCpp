#include <gui_controller.hpp>
#include <main_frame.hpp>

#include <mudcore/display_line.hpp>

namespace genesis::gui {

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

    ioThread_ = std::thread([this]() { ioContext_.run(); });

    timer_.SetOwner(this);
    Bind(wxEVT_TIMER, &GuiController::onTimer, this, timer_.GetId());
    timer_.Start(30);
}

void GuiController::shutdown() {
    timer_.Stop();
    Unbind(wxEVT_TIMER, &GuiController::onTimer, this, timer_.GetId());

    session_.disconnect();

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
}

void GuiController::submitCommand(const std::string_view command) {
    session_.sendCommand(command);
}

void GuiController::onTimer(wxTimerEvent& /*event*/) {
    const mudcore::PollResult result = session_.poll();

    for (const mudcore::DisplayLine& line : result.lines) {
        routeDisplayLine(line);
    }

    if (result.connectionPhase != lastPhase_) {
        lastPhase_ = result.connectionPhase;
        frame_.setConnectionPhase(result.connectionPhase);
        frame_.connectionFooter().setPhase(result.connectionPhase);
    }

    if (result.stateChanged) {
        frame_.vitalsBar().refresh(session_.gameState());
        frame_.magicMapPanel().setMapText(session_.gameState().room().map);
    }
}

void GuiController::routeDisplayLine(const mudcore::DisplayLine& line) {
    switch (line.sink) {
    case mudcore::OutputSink::Main:
        frame_.mainDisplay().appendLine(line.text);
        break;
    case mudcore::OutputSink::Comms:
        frame_.comms().appendLine(line.text);
        break;
    case mudcore::OutputSink::System:
        frame_.systemLog().appendLine(line.text);
        break;
    }
}

} // namespace genesis::gui
