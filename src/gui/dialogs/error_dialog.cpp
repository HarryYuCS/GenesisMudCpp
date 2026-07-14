#include <dialogs/error_dialog.hpp>

namespace genesis::gui {

void ErrorDialog::show(wxWindow* parent, const wxString& message) {
    wxMessageDialog dialog(parent, message, "Error", wxOK | wxICON_ERROR | wxCENTRE);
    dialog.ShowModal();
}

} // namespace genesis::gui
