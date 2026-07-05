#ifndef GENESIS_GUI_MAIN_FRAME_HPP
#define GENESIS_GUI_MAIN_FRAME_HPP

#include <controls/input_bar.hpp>
#include <controls/main_display.hpp>
#include <mudcore/connection_life_cycle.hpp>
#include <panels/connection_footer.hpp>
#include <panels/comms.hpp>
#include <panels/magic_map_panel.hpp>
#include <panels/right_column.hpp>
#include <panels/system_log.hpp>
#include <panels/vitals_bar.hpp>

#include <wx/wx.h>

namespace genesis::gui {

/**
 * @brief Top-level application window; owns macro layout and child panel accessors.
 */
class MainClientFrame : public wxFrame {
public:
    MainClientFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
    ~MainClientFrame() override = default;

    MainDisplay& mainDisplay();
    RightColumnPanel& rightColumn();
    CommsPanel& comms();
    MagicMapPanel& magicMapPanel();
    InputBar& inputBar();
    VitalsBarPanel& vitalsBar();
    SystemLogPanel& systemLog();
    ConnectionFooterPanel& connectionFooter();

    void setConnectionPhase(mudcore::ConnectionPhase phase);

private:
    wxString baseTitle_;
    mudcore::ConnectionPhase lastPhase_{mudcore::ConnectionPhase::Disconnected};

    MainDisplay* mainDisplay_{nullptr};
    RightColumnPanel* rightColumn_{nullptr};
    InputBar* inputBar_{nullptr};
    VitalsBarPanel* vitalsBar_{nullptr};
    ConnectionFooterPanel* connectionFooter_{nullptr};
};

} // namespace genesis::gui

#endif // GENESIS_GUI_MAIN_FRAME_HPP
