#include <menu/menu_bar.hpp>

namespace genesis::gui {

MenuBar::MenuBar() {
    auto* fileMenu = new wxMenu();
    fileMenu->Append(static_cast<int>(MenuId::Connect), "&Connect");
    fileMenu->Append(static_cast<int>(MenuId::Disconnect), "&Disconnect");
    fileMenu->AppendSeparator();
    fileMenu->Append(static_cast<int>(MenuId::Quit), "E&xit");
    Append(fileMenu, "&File");
}

} // namespace genesis::gui
