#ifndef GENESIS_GUI_DIALOGS_SETTINGS_DIALOG_HPP
#define GENESIS_GUI_DIALOGS_SETTINGS_DIALOG_HPP

#include <wx/wx.h>

namespace genesis::gui {

class SettingsDialog : public wxDialog {
public:
    explicit SettingsDialog(wxWindow* parent);

    bool showModal();

private:
    void onSave(wxCommandEvent& event);

    wxTextCtrl* hostField_{nullptr};
    wxTextCtrl* portField_{nullptr};
    wxCheckBox* debugLoggingCheck_{nullptr};
};

} // namespace genesis::gui

#endif // GENESIS_GUI_DIALOGS_SETTINGS_DIALOG_HPP
