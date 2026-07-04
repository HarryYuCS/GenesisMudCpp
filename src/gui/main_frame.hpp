#ifndef GENESIS_GUI_MAIN_FRAME_HPP
#define GENESIS_GUI_MAIN_FRAME_HPP

#include <wx/wx.h>

namespace genesis::gui {

class MainClientFrame : public wxFrame {
public:
    MainClientFrame(const wxString& title, const wxPoint& pos, const wxSize& size) :
        wxFrame(title, wxID_ANY, pos, size),
        mainHorizontalSizer(new wxGridSizer(2, 0, 0))
    {
        SetSizer(mainHorizontalSizer);
        // TODO : add child windows to mainHorizontalSizer
    }

    ~MainClientFrame() = default;

private:
    wxGridSizer *mainHorizontalSizer;

    // TODO: add child windows from other headers
};

} // namespace genesis::gui

#endif // GENESIS_GUI_MAIN_FRAME_HPP