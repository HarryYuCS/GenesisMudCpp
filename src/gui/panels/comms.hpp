#ifndef GENESIS_GUI_PANELS_COMMS_HPP
#define GENESIS_GUI_PANELS_COMMS_HPP

#include <wx/wx.h>

#include <string_view>

namespace genesis::gui {

class CommsPanel : public wxPanel {
public:
    explicit CommsPanel(wxWindow* parent);
    ~CommsPanel() override = default;

    CommsPanel(const CommsPanel&) = delete;
    CommsPanel& operator=(const CommsPanel&) = delete;

    void appendLine(std::string_view text);

private:
    wxTextCtrl* log_{nullptr};
};

} // namespace genesis::gui

#endif // GENESIS_GUI_PANELS_COMMS_HPP
