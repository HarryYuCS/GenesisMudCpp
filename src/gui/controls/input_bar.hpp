#ifndef GENESIS_GUI_CONTROLS_INPUT_BAR_HPP
#define GENESIS_GUI_CONTROLS_INPUT_BAR_HPP

#include <wx/wx.h>

#include <functional>
#include <string_view>

namespace genesis::gui {

class InputBar : public wxTextCtrl {
public:
    explicit InputBar(wxWindow* parent);
    ~InputBar() override = default;

    InputBar(const InputBar&) = delete;
    InputBar& operator=(const InputBar&) = delete;

    void setOnSubmit(std::function<void(std::string_view)> callback);

private:
    void onEnter(wxCommandEvent& event);

    std::function<void(std::string_view)> onSubmit_;
};

} // namespace genesis::gui

#endif // GENESIS_GUI_CONTROLS_INPUT_BAR_HPP
