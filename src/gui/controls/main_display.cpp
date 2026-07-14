#include <controls/main_display.hpp>

namespace genesis::gui {

namespace {

wxFont monospaceDisplayFont() {
    return wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
}

} // namespace

MainDisplay::MainDisplay(wxWindow* parent)
    : wxTextCtrl(
          parent,
          wxID_ANY,
          wxEmptyString,
          wxDefaultPosition,
          wxDefaultSize,
          wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH2) {
    SetFont(monospaceDisplayFont());
    SetDefaultStyle(wxTextAttr(*wxBLACK));
}

void MainDisplay::append(const std::string_view text) {
    SetDefaultStyle(wxTextAttr(*wxBLACK));
    AppendText(wxString::FromUTF8(text.data(), static_cast<int>(text.size())));
    ShowPosition(GetLastPosition());
}

void MainDisplay::appendStyled(const std::string_view text, const wxColour& colour) {
    SetDefaultStyle(wxTextAttr(colour));
    AppendText(wxString::FromUTF8(text.data(), static_cast<int>(text.size())));
    SetDefaultStyle(wxTextAttr(*wxBLACK));
    ShowPosition(GetLastPosition());
}

void MainDisplay::clear() {
    Clear();
}

} // namespace genesis::gui
