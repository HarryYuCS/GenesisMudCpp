#ifndef GENESIS_GUI_PANELS_CONNECTION_FOOTER_HPP
#define GENESIS_GUI_PANELS_CONNECTION_FOOTER_HPP

#include <mudcore/connection_life_cycle.hpp>

#include <wx/wx.h>

#include <functional>

namespace genesis::gui {

class ConnectionFooterPanel : public wxPanel {
public:
    explicit ConnectionFooterPanel(wxWindow* parent);
    ~ConnectionFooterPanel() override = default;

    ConnectionFooterPanel(const ConnectionFooterPanel&) = delete;
    ConnectionFooterPanel& operator=(const ConnectionFooterPanel&) = delete;

    void setPhase(mudcore::ConnectionPhase phase);
    void setOnConnect(std::function<void()> callback);
    void setOnDisconnect(std::function<void()> callback);
    void setOnSettings(std::function<void()> callback);

private:
    void onConnectButton(wxCommandEvent& event);
    void onSettingsButton(wxCommandEvent& event);

    wxStaticText* phaseLabel_{nullptr};
    wxButton* connectButton_{nullptr};
    mudcore::ConnectionPhase phase_{mudcore::ConnectionPhase::Disconnected};
    std::function<void()> onConnect_;
    std::function<void()> onDisconnect_;
    std::function<void()> onSettings_;
};

} // namespace genesis::gui

#endif // GENESIS_GUI_PANELS_CONNECTION_FOOTER_HPP
