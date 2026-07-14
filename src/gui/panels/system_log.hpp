#ifndef GENESIS_GUI_PANELS_SYSTEM_LOG_HPP
#define GENESIS_GUI_PANELS_SYSTEM_LOG_HPP

#include <wx/wx.h>

#include <string_view>

namespace genesis::gui {

class SystemLogPanel : public wxPanel {
public:
    explicit SystemLogPanel(wxWindow* parent);
    ~SystemLogPanel() override = default;

    SystemLogPanel(const SystemLogPanel&) = delete;
    SystemLogPanel& operator=(const SystemLogPanel&) = delete;

    void append(std::string_view text);
    void clear();

private:
    wxTextCtrl* log_{nullptr};
};

} // namespace genesis::gui

#endif // GENESIS_GUI_PANELS_SYSTEM_LOG_HPP
