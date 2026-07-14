#ifndef GENESIS_GUI_CONTROLS_INPUT_BAR_HPP
#define GENESIS_GUI_CONTROLS_INPUT_BAR_HPP

#include <wx/wx.h>

#include <functional>
#include <string>
#include <string_view>
#include <vector>

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
    void onKeyDown(wxKeyEvent& event);
    void pushHistory(const std::string& command);
    void showHistoryEntry(int index);

    std::function<void(std::string_view)> onSubmit_;
    std::vector<std::string> history_;
    int historyIndex_{-1}; ///< -1 means "editing current / past end"
};

} // namespace genesis::gui

#endif // GENESIS_GUI_CONTROLS_INPUT_BAR_HPP
