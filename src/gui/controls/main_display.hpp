#ifndef GENESIS_GUI_CONTROLS_MAIN_DISPLAY_HPP
#define GENESIS_GUI_CONTROLS_MAIN_DISPLAY_HPP

#include <wx/wx.h>

#include <string_view>

namespace genesis::gui {

/**
 * @brief Read-only text control for the main display (most game output).
 */
class MainDisplay : public wxTextCtrl {
public:
    explicit MainDisplay(wxWindow* parent);
    ~MainDisplay() override = default;

    MainDisplay(const MainDisplay&) = delete;
    MainDisplay& operator=(const MainDisplay&) = delete;

    void append(std::string_view text);
    void appendStyled(std::string_view text, const wxColour& colour);
    void clear();
};

} // namespace genesis::gui

#endif // GENESIS_GUI_CONTROLS_MAIN_DISPLAY_HPP
