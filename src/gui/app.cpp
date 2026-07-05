#include <gui_controller.hpp>
#include <main_frame.hpp>

#include <wx/wx.h>

#include <memory>

namespace genesis::gui {

class GenesisApp : public wxApp {
public:
    bool OnInit() override;
    int OnExit() override;

private:
    MainClientFrame* frame_{nullptr};
    std::unique_ptr<GuiController> controller_;
};

} // namespace genesis::gui

wxIMPLEMENT_APP(genesis::gui::GenesisApp);

namespace genesis::gui {

bool GenesisApp::OnInit() {
    frame_ = new MainClientFrame("Genesis MUD Client", wxDefaultPosition, wxSize(1024, 768));
    controller_ = std::make_unique<GuiController>(*frame_);
    controller_->start();
    frame_->Show(true);
    return true;
}

int GenesisApp::OnExit() {
    if (controller_) {
        controller_->shutdown();
        controller_.reset();
    }
    return wxApp::OnExit();
}

} // namespace genesis::gui
