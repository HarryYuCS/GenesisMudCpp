#include <panels/map_notebook.hpp>

namespace genesis::gui {

MapNotebookPanel::MapNotebookPanel(wxWindow* parent)
    : wxPanel(parent) {
    auto* notebook = new wxNotebook(this, wxID_ANY);

    magicMapPanel_ = new MagicMapPanel(notebook);
    systemLogPanel_ = new SystemLogPanel(notebook);

    notebook->AddPage(magicMapPanel_, "MAGIC MAP");
    notebook->AddPage(systemLogPanel_, "SYSTEM LOG");

    auto* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(notebook, 1, wxEXPAND);
    SetSizer(sizer);
}

MagicMapPanel& MapNotebookPanel::magicMapPanel() {
    return *magicMapPanel_;
}

SystemLogPanel& MapNotebookPanel::systemLogPanel() {
    return *systemLogPanel_;
}

} // namespace genesis::gui
