#include <controls/input_bar.hpp>

namespace genesis::gui {

InputBar::InputBar(wxWindow* parent)
    : wxTextCtrl(
          parent,
          wxID_ANY,
          wxEmptyString,
          wxDefaultPosition,
          wxDefaultSize,
          wxTE_PROCESS_ENTER) {
    Bind(wxEVT_TEXT_ENTER, &InputBar::onEnter, this);
}

void InputBar::setOnSubmit(std::function<void(std::string_view)> callback) {
    onSubmit_ = std::move(callback);
}

void InputBar::onEnter(wxCommandEvent& /*event*/) {
    if (!onSubmit_) {
        return;
    }

    const wxString value = GetValue();
    if (value.IsEmpty()) {
        return;
    }

    const std::string command = value.ToUTF8().data();
    onSubmit_(command);
    Clear();
}

} // namespace genesis::gui
