#include <controls/magic_map.hpp>

namespace genesis::gui {

MagicMap::MagicMap(wxWindow* parent)
    : wxTextCtrl(
          parent,
          wxID_ANY,
          wxEmptyString,
          wxDefaultPosition,
          wxDefaultSize,
          wxTE_MULTILINE | wxTE_READONLY) {}

void MagicMap::setMapText(const std::string_view text) {
    SetValue(wxString::FromUTF8(text.data(), static_cast<int>(text.size())));
}

} // namespace genesis::gui
