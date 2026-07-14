#ifndef GENESIS_GUI_PANELS_VITALS_BAR_HPP
#define GENESIS_GUI_PANELS_VITALS_BAR_HPP

#include <mudcore/game_state.hpp>

#include <wx/wx.h>

namespace genesis::gui {

class VitalsBarPanel : public wxPanel {
public:
    explicit VitalsBarPanel(wxWindow* parent);
    ~VitalsBarPanel() override = default;

    VitalsBarPanel(const VitalsBarPanel&) = delete;
    VitalsBarPanel& operator=(const VitalsBarPanel&) = delete;

    void refresh(const mudcore::GameState& state);
    void clear();

private:
    class ColoredVitalBar;

    ColoredVitalBar* health_{nullptr};
    ColoredVitalBar* mana_{nullptr};
    ColoredVitalBar* stamina_{nullptr};
};

} // namespace genesis::gui

#endif // GENESIS_GUI_PANELS_VITALS_BAR_HPP
