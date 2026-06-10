#include <wx/wx.h>

class GenesisApp : public wxApp {
public:
    bool OnInit() override;
};

IMPLEMENT_APP(GenesisApp)

bool GenesisApp::OnInit() {
    wxFrame* frame = new wxFrame(nullptr, wxID_ANY, "Genesis MUD Client");
    frame->Show(true);
    return true;
}