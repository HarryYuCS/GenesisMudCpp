#ifndef GENESIS_GUI_GUI_CONTROLLER_HPP
#define GENESIS_GUI_GUI_CONTROLLER_HPP

#include <mudcore/connection_life_cycle.hpp>
#include <mudcore/session.hpp>

#include <wx/wx.h>

#include <boost/asio/io_context.hpp>
#include <boost/asio/executor_work_guard.hpp>

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <thread>

namespace genesis::gui {

class MainClientFrame;

/**
 * @brief Bridges mudcore::Session to MainClientFrame on the wx main thread.
 *
 * Owns the network io_context thread and a wxTimer that drives session.poll().
 */
class GuiController : public wxEvtHandler {
public:
    explicit GuiController(MainClientFrame& frame);
    ~GuiController() override;

    GuiController(const GuiController&) = delete;
    GuiController& operator=(const GuiController&) = delete;

    void start();
    void shutdown();

    void connect(const std::string& host, std::uint16_t port);
    void disconnect();
    void submitCommand(std::string_view command);

private:
    void wireFrame();
    void wireInputBar();
    void wireMenuEvents();
    void wireFooterEvents();
    void wireCloseEvent();

    void showConnectDialog();
    void showSettingsDialog();
    void onMenu(wxCommandEvent& event);
    void onClose(wxCloseEvent& event);
    void onTimer(wxTimerEvent& event);
    void routeDisplayLine(const mudcore::DisplayLine& line);
    void updateInputEnabled(mudcore::ConnectionPhase phase);
    void clearAllDisplays();
    void applySettings();
    void logPhaseChange(mudcore::ConnectionPhase from, mudcore::ConnectionPhase to);

    MainClientFrame& frame_;
    boost::asio::io_context ioContext_;
    mudcore::Session session_;
    std::optional<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>> workGuard_;
    std::thread ioThread_;
    wxTimer timer_;
    mudcore::ConnectionPhase lastPhase_{mudcore::ConnectionPhase::Disconnected};
    std::string lastShownError_;
    bool wired_{false};
    bool shutDown_{false};
    bool debugLogging_{false};
};

} // namespace genesis::gui

#endif // GENESIS_GUI_GUI_CONTROLLER_HPP
