#include <controls/input_bar.hpp>

namespace genesis::gui {

namespace {

constexpr std::size_t kMaxHistory = 100;

/** @brief Single-letter cardinal moves are too noisy for Up/Down recall. */
bool isHistoryBlacklisted(const std::string& command) {
    return command == "n" || command == "s" || command == "e" || command == "w";
}

} // namespace

InputBar::InputBar(wxWindow* parent)
    : wxTextCtrl(
          parent,
          wxID_ANY,
          wxEmptyString,
          wxDefaultPosition,
          wxDefaultSize,
          wxTE_PROCESS_ENTER) {
    Bind(wxEVT_TEXT_ENTER, &InputBar::onEnter, this);
    Bind(wxEVT_KEY_DOWN, &InputBar::onKeyDown, this);
}

void InputBar::setOnSubmit(std::function<void(std::string_view)> callback) {
    onSubmit_ = std::move(callback);
}

void InputBar::pushHistory(const std::string& command) {
    if (command.empty() || isHistoryBlacklisted(command)) {
        return;
    }
    if (!history_.empty() && history_.back() == command) {
        return;
    }
    history_.push_back(command);
    if (history_.size() > kMaxHistory) {
        history_.erase(history_.begin());
    }
}

void InputBar::showHistoryEntry(const int index) {
    if (index < 0 || index >= static_cast<int>(history_.size())) {
        return;
    }
    historyIndex_ = index;
    const std::string& entry = history_[static_cast<std::size_t>(index)];
    ChangeValue(wxString::FromUTF8(entry.data(), static_cast<int>(entry.size())));
    SetInsertionPointEnd();
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
    pushHistory(command);
    historyIndex_ = -1;
    SetSelection(0, GetLastPosition());
}

void InputBar::onKeyDown(wxKeyEvent& event) {
    const int key = event.GetKeyCode();

    if (key == WXK_UP) {
        if (history_.empty()) {
            return;
        }
        if (historyIndex_ < 0) {
            showHistoryEntry(static_cast<int>(history_.size()) - 1);
        } else if (historyIndex_ > 0) {
            showHistoryEntry(historyIndex_ - 1);
        }
        return;
    }

    if (key == WXK_DOWN) {
        if (history_.empty() || historyIndex_ < 0) {
            return;
        }
        if (historyIndex_ + 1 < static_cast<int>(history_.size())) {
            showHistoryEntry(historyIndex_ + 1);
        } else {
            historyIndex_ = -1;
            ChangeValue(wxEmptyString);
        }
        return;
    }

    if (key != WXK_UP && key != WXK_DOWN && key != WXK_RETURN && key != WXK_NUMPAD_ENTER) {
        historyIndex_ = -1;
    }

    event.Skip();
}

} // namespace genesis::gui
