#include "screen_toolhead_settings_common.hpp"

using namespace screen_toolhead_settings;

bool screen_toolhead_settings::msgbox_confirm_change(Toolhead toolhead, bool &confirmed_before) {
    // Confirmed before - don't ask again
    if (confirmed_before) {
        return true;
    }

    // Single toolhead change -> no need to confirm
    if (toolhead != all_toolheads) {
        return true;
    }

    const bool confirmed = MsgBoxQuestion(_("This change will apply to all toolheads. Do you want to continue?"), Responses_YesNo) == Response::Yes;
    confirmed_before = confirmed;
    return confirmed;
}

// MI_TOOLHEAD_SPECIFIC_SPIN
// ============================================
void MI_TOOLHEAD_SPECIFIC_SPIN::update() {
    set_value(read_value().value_or(*config().special_value));
}
void MI_TOOLHEAD_SPECIFIC_SPIN::OnClick() {
    if (value() == config().special_value) {
        return;
    }

    if (msgbox_confirm_change(toolhead(), user_already_confirmed_changes_)) {
        store_value(value());
    } else {
        update();
    }
}

// MI_TOOLHEAD_SPECIFIC_TOGGLE
// ============================================
void MI_TOOLHEAD_SPECIFIC_TOGGLE::update() {
    set_value(read_value().transform(Tristate::from_bool).value_or(Tristate::other), false);
}
void MI_TOOLHEAD_SPECIFIC_TOGGLE::toggled(Tristate) {
    if (value() == Tristate::other) {
        return;
    }

    if (msgbox_confirm_change(toolhead(), user_already_confirmed_changes_)) {
        store_value(value());
    } else {
        update();
    }
}
