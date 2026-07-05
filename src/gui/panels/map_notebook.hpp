#ifndef GENESIS_GUI_PANELS_MAP_NOTEBOOK_HPP
#define GENESIS_GUI_PANELS_MAP_NOTEBOOK_HPP

#include <panels/magic_map_panel.hpp>
#include <panels/system_log.hpp>

#include <wx/notebook.h>
#include <wx/wx.h>

namespace genesis::gui {

class MapNotebookPanel : public wxPanel {
public:
    explicit MapNotebookPanel(wxWindow* parent);
    ~MapNotebookPanel() override = default;

    MapNotebookPanel(const MapNotebookPanel&) = delete;
    MapNotebookPanel& operator=(const MapNotebookPanel&) = delete;

    MagicMapPanel& magicMapPanel();
    SystemLogPanel& systemLogPanel();

private:
    MagicMapPanel* magicMapPanel_{nullptr};
    SystemLogPanel* systemLogPanel_{nullptr};
};

} // namespace genesis::gui

#endif // GENESIS_GUI_PANELS_MAP_NOTEBOOK_HPP
