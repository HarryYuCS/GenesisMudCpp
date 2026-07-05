#ifndef GENESIS_GUI_PANELS_RIGHT_COLUMN_HPP
#define GENESIS_GUI_PANELS_RIGHT_COLUMN_HPP

#include <panels/comms.hpp>
#include <panels/map_notebook.hpp>

#include <wx/wx.h>

namespace genesis::gui {

class RightColumnPanel : public wxPanel {
public:
    explicit RightColumnPanel(wxWindow* parent);
    ~RightColumnPanel() override = default;

    RightColumnPanel(const RightColumnPanel&) = delete;
    RightColumnPanel& operator=(const RightColumnPanel&) = delete;

    MapNotebookPanel& mapNotebook();
    CommsPanel& comms();

private:
    MapNotebookPanel* mapNotebook_{nullptr};
    CommsPanel* comms_{nullptr};
};

} // namespace genesis::gui

#endif // GENESIS_GUI_PANELS_RIGHT_COLUMN_HPP
