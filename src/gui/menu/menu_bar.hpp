#ifndef GENESIS_GUI_MENU_MENU_BAR_HPP
#define GENESIS_GUI_MENU_MENU_BAR_HPP

#include <mudcore/connection_life_cycle.hpp>

#include <wx/wx.h>

namespace genesis::gui {

enum class MenuId : int {
    Connect = 4001,
    Disconnect = 4002,
    Quit = wxID_EXIT,
};

class MenuBar : public wxMenuBar {
public:
    MenuBar();
    ~MenuBar() override = default;

    MenuBar(const MenuBar&) = delete;
    MenuBar& operator=(const MenuBar&) = delete;

    void updateForPhase(mudcore::ConnectionPhase phase);

private:
    wxMenuItem* connectItem_{nullptr};
    wxMenuItem* disconnectItem_{nullptr};
};

} // namespace genesis::gui

#endif // GENESIS_GUI_MENU_MENU_BAR_HPP
