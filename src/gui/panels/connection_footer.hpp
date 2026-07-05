#ifndef GENESIS_GUI_PANELS_CONNECTION_FOOTER_HPP
#define GENESIS_GUI_PANELS_CONNECTION_FOOTER_HPP

#include <mudcore/connection_life_cycle.hpp>

#include <wx/wx.h>

namespace genesis::gui {

class ConnectionFooterPanel : public wxPanel {
public:
    explicit ConnectionFooterPanel(wxWindow* parent);
    ~ConnectionFooterPanel() override = default;

    ConnectionFooterPanel(const ConnectionFooterPanel&) = delete;
    ConnectionFooterPanel& operator=(const ConnectionFooterPanel&) = delete;

    void setPhase(mudcore::ConnectionPhase phase);

private:
    wxStaticText* phaseLabel_{nullptr};
};

} // namespace genesis::gui

#endif // GENESIS_GUI_PANELS_CONNECTION_FOOTER_HPP
