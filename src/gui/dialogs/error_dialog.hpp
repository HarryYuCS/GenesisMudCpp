#ifndef GENESIS_GUI_DIALOGS_ERROR_DIALOG_HPP
#define GENESIS_GUI_DIALOGS_ERROR_DIALOG_HPP

#include <wx/wx.h>

namespace genesis::gui {

class ErrorDialog {
public:
    static void show(wxWindow* parent, const wxString& message);
};

} // namespace genesis::gui

#endif // GENESIS_GUI_DIALOGS_ERROR_DIALOG_HPP
