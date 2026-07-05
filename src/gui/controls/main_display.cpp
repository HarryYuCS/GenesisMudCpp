#include <controls/main_display.hpp>

namespace genesis::gui {

MainDisplay::MainDisplay(wxWindow* parent)
    : wxTextCtrl(
          parent,
          wxID_ANY,
          wxEmptyString,
          wxDefaultPosition,
          wxDefaultSize,
          wxTE_MULTILINE | wxTE_READONLY) {}

void MainDisplay::appendLine(const std::string_view text) {
    AppendText(wxString::FromUTF8(text.data(), static_cast<int>(text.size())));
    AppendText("\n");
}

} // namespace genesis::gui
