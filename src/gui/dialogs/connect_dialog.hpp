#ifndef GENESIS_GUI_DIALOGS_CONNECT_DIALOG_HPP
#define GENESIS_GUI_DIALOGS_CONNECT_DIALOG_HPP

#include <wx/wx.h>

#include <cstdint>
#include <string>

namespace genesis::gui {

class ConnectDialog : public wxDialog {
public:
    explicit ConnectDialog(wxWindow* parent);

    bool showModal();
    const std::string& host() const;
    std::uint16_t port() const;

private:
    void onOk(wxCommandEvent& event);

    wxTextCtrl* hostField_{nullptr};
    wxTextCtrl* portField_{nullptr};
    std::string host_;
    std::uint16_t port_{3011};
};

} // namespace genesis::gui

#endif // GENESIS_GUI_DIALOGS_CONNECT_DIALOG_HPP
