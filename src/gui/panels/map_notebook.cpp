#include <panels/map_notebook.hpp>

namespace genesis::gui {

MapNotebookPanel::MapNotebookPanel(wxWindow* parent)
    : wxPanel(parent) {
    notebook_ = new wxNotebook(this, wxID_ANY);

    magicMapPanel_ = new MagicMapPanel(notebook_);
    systemLogPanel_ = new SystemLogPanel(notebook_);

    notebook_->AddPage(magicMapPanel_, "MAGIC MAP");
    notebook_->AddPage(systemLogPanel_, "SYSTEM LOG");

    auto* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(notebook_, 1, wxEXPAND);
    SetSizer(sizer);
}

MagicMapPanel& MapNotebookPanel::magicMapPanel() {
    return *magicMapPanel_;
}

SystemLogPanel& MapNotebookPanel::systemLogPanel() {
    return *systemLogPanel_;
}

void MapNotebookPanel::selectSystemLogTab() {
    const int pageIndex = notebook_->FindPage(systemLogPanel_);
    if (pageIndex != wxNOT_FOUND) {
        notebook_->SetSelection(static_cast<size_t>(pageIndex));
    }
}

} // namespace genesis::gui
