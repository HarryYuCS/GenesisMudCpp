#include <panels/vitals_bar.hpp>

#include <mudcore/string_utils.hpp>

#include <wx/dcbuffer.h>

#include <algorithm>
#include <span>
#include <string_view>

namespace genesis::gui {

namespace {

constexpr int kMinFillPercent = 5;
constexpr int kMaxFillPercent = 100;

int percentFromOrderedScale(std::string_view level, std::span<const std::string_view> worstToBest) {
    if (level.empty()) {
        return 0;
    }
    if (worstToBest.size() == 1) {
        return genesis::mudcore::equalsIgnoreCase(level, worstToBest.front()) ? kMaxFillPercent : 50;
    }

    for (std::size_t index = 0; index < worstToBest.size(); ++index) {
        if (genesis::mudcore::equalsIgnoreCase(level, worstToBest[index])) {
            const int span = kMaxFillPercent - kMinFillPercent;
            return kMinFillPercent
                + static_cast<int>((index * span) / (worstToBest.size() - 1));
        }
    }

    return 50;
}

// Genesis vital scales (worst → best), from Char.Vitals.Levels.
constexpr std::string_view kHealthLevels[] = {
    "at death's door",
    "barely alive",
    "terribly hurt",
    "in a very bad shape",
    "in agony",
    "in a bad shape",
    "very hurt",
    "suffering",
    "hurt",
    "aching",
    "somewhat hurt",
    "slightly hurt",
    "sore",
    "feeling well",
    "feeling very well",
};

constexpr std::string_view kManaLevels[] = {
    "in a vegetable state",
    "exhausted",
    "worn down",
    "indisposed",
    "in a bad shape",
    "very degraded",
    "rather degraded",
    "degraded",
    "somewhat degraded",
    "slightly degraded",
    "in full vigour",
};

// Listed best-to-worst in-game; reversed here to worst → best.
constexpr std::string_view kFatigueLevels[] = {
    "exhausted",
    "tired",
    "weary",
    "alert",
};

int healthPercent(const std::string_view level) {
    return percentFromOrderedScale(level, kHealthLevels);
}

int manaPercent(const std::string_view level) {
    return percentFromOrderedScale(level, kManaLevels);
}

int staminaPercent(const std::string_view level) {
    return percentFromOrderedScale(level, kFatigueLevels);
}

} // namespace

class VitalsBarPanel::ColoredVitalBar : public wxPanel {
public:
    ColoredVitalBar(wxWindow* parent, const wxString& title, const wxColour& fillColor)
        : wxPanel(parent)
        , title_(title)
        , fillColor_(fillColor) {
        SetMinSize(wxSize(-1, 28));
        SetBackgroundStyle(wxBG_STYLE_PAINT);
        Bind(wxEVT_PAINT, &ColoredVitalBar::onPaint, this);
    }

    void setLevel(std::string_view levelText, int percent) {
        percent_ = std::clamp(percent, 0, 100);
        levelText_ = wxString::FromUTF8(levelText.data(), static_cast<int>(levelText.size()));
        Refresh();
    }

private:
    void onPaint(wxPaintEvent& /*event*/) {
        wxAutoBufferedPaintDC dc(this);
        const wxSize size = GetClientSize();

        dc.SetBrush(wxBrush(wxColour(45, 45, 45)));
        dc.SetPen(*wxTRANSPARENT_PEN);
        dc.DrawRectangle(0, 0, size.x, size.y);

        const int fillWidth = (size.x * percent_) / 100;
        if (fillWidth > 0) {
            dc.SetBrush(wxBrush(fillColor_));
            dc.DrawRectangle(0, 0, fillWidth, size.y);
        }

        dc.SetTextForeground(*wxWHITE);
        dc.SetFont(GetFont().Bold());
        const wxString label = title_ + "  " + levelText_;
        dc.DrawText(label, 6, (size.y - dc.GetTextExtent(label).y) / 2);
    }

    wxString title_;
    wxString levelText_;
    wxColour fillColor_;
    int percent_{0};
};

VitalsBarPanel::VitalsBarPanel(wxWindow* parent)
    : wxPanel(parent) {
    health_ = new ColoredVitalBar(this, "HEALTH", wxColour(200, 50, 50));
    mana_ = new ColoredVitalBar(this, "MANA", wxColour(50, 110, 220));
    stamina_ = new ColoredVitalBar(this, "STAMINA", wxColour(50, 180, 70));

    auto* sizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(health_, 1, wxEXPAND | wxALL, 4);
    sizer->Add(mana_, 1, wxEXPAND | wxALL, 4);
    sizer->Add(stamina_, 1, wxEXPAND | wxALL, 4);
    SetSizer(sizer);
}

void VitalsBarPanel::refresh(const mudcore::GameState& state) {
    health_->setLevel(state.healthLevel(), healthPercent(state.healthLevel()));
    mana_->setLevel(state.manaLevel(), manaPercent(state.manaLevel()));
    stamina_->setLevel(state.fatigueLevel(), staminaPercent(state.fatigueLevel()));
}

} // namespace genesis::gui
