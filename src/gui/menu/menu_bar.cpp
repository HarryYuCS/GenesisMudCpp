#include <menu/menu_bar.hpp>

namespace genesis::gui {

MenuBar::MenuBar() {
    auto* fileMenu = new wxMenu();
    connectItem_ = fileMenu->Append(static_cast<int>(MenuId::Connect), "&Connect");
    disconnectItem_ = fileMenu->Append(static_cast<int>(MenuId::Disconnect), "&Disconnect");
    fileMenu->AppendSeparator();
    fileMenu->Append(static_cast<int>(MenuId::Quit), "E&xit");
    Append(fileMenu, "&File");
    updateForPhase(mudcore::ConnectionPhase::Disconnected);
}

void MenuBar::updateForPhase(const mudcore::ConnectionPhase phase) {
    const bool disconnected = phase == mudcore::ConnectionPhase::Disconnected;
    const bool connecting = phase == mudcore::ConnectionPhase::Connecting;

    connectItem_->Enable(disconnected);
    disconnectItem_->Enable(!disconnected && !connecting);
}

} // namespace genesis::gui
