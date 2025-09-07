/**
 * @file MItem_print_head_profiles.cpp
 */

#include "MItem_print_head_profiles.hpp"
#include "screen_menu_print_head_profiles.hpp"
#include "ScreenHandler.hpp"
#include <ScreenFactory.hpp>
#include <window_msgbox.hpp>
#include <MItem_tools.hpp> // gui_try_gcode_with_msg
#include "config_store/store_instance.hpp"
#include "i18n.h"
#include "lang/string_view_utf8.hpp"
#include <Marlin/src/feature/input_shaper/input_shaper_config.hpp>
#include <cmath>
#include <string>

float Revosix = 1.0f; // temp freq

// Main menu entry
MI_PRINT_HEAD_PROFILES::MI_PRINT_HEAD_PROFILES()
    : IWindowMenuItem(_(label), nullptr, is_enabled_t::yes, is_hidden_t::no) {}

void MI_PRINT_HEAD_PROFILES::click(IWindowMenu &window_menu) {
    (void)window_menu;
    Screens::Access()->Open(ScreenFactory::Screen<ScreenMenuPrintHeadProfiles>);
}

// Original Prusa -> restore defaults for X-axis Input Shaper
MI_HEAD_ORIGINAL_PRUSA::MI_HEAD_ORIGINAL_PRUSA()
    : IWindowMenuItem(_(label), nullptr, is_enabled_t::yes, is_hidden_t::no) {}

void MI_HEAD_ORIGINAL_PRUSA::click(IWindowMenu &window_menu) {
    (void)window_menu;
    if (MsgBoxQuestion(_("Apply Original Prusa X-axis Input Shaper defaults?"), Responses_YesNo) != Response::Yes) {
        return;
    }

    // Only restore X-axis to default; leave Y as-is
    auto &store = config_store();
    store.input_shaper_axis_x_config.set_to_default();
    store.input_shaper_axis_x_enabled.set(true);

    // Make the input shaper reload config from config_store
    gui_try_gcode_with_msg("M9200");

    MsgBoxInfo(_("Applied. You can fine-tune in Input Shaper menu."), Responses_Ok);
}

// Revo Six
MI_HEAD_REVO_SIX::MI_HEAD_REVO_SIX()
    : IWindowMenuItem(_(label), nullptr, is_enabled_t::yes, is_hidden_t::no) {}

void MI_HEAD_REVO_SIX::click(IWindowMenu &window_menu) {
    (void)window_menu;
    if (MsgBoxQuestion(_("Apply Revo Six print head profile?"), Responses_YesNo) != Response::Yes) {
        return;
    }

    auto axis_x = config_store().input_shaper_axis_x_config.get();
    axis_x.frequency = Revosix;
    config_store().input_shaper_axis_x_config.set(axis_x);
    config_store().input_shaper_axis_x_enabled.set(true);

    // Make the input shaper reload config from config_store
    gui_try_gcode_with_msg("M9200");

    MsgBoxInfo(_("Revo Six profile applied. You can fine-tune later."), Responses_Ok);
}

// ---------------- Current Print Head profile (info row) ----------------

static bool approx_equal(float a, float b, float eps = 0.01f) {
    return std::fabs(a - b) < eps;
}

const char *MI_HEAD_CURRENT::get_current_head_profile_name() {
    auto &store = config_store();
    const bool enabled = store.input_shaper_axis_x_enabled.get();
    if (!enabled) {
        return "Disabled";
    }

    const auto cfg = store.input_shaper_axis_x_config.get();

    // Recognize Revo Six preset (our temporary definition)
    if (approx_equal(cfg.frequency, Revosix)) {
        return "Revo Six";
    }

    // Recognize Original Prusa defaults
    using namespace input_shaper;
    if (cfg.type == axis_x_default.type
        && approx_equal(cfg.frequency, axis_x_default.frequency)
        && approx_equal(cfg.damping_ratio, axis_x_default.damping_ratio)
        && approx_equal(cfg.vibration_reduction, axis_x_default.vibration_reduction)) {
        return "Original Prusa";
    }

    return "Custom";
}

MI_HEAD_CURRENT::MI_HEAD_CURRENT()
    : WI_INFO_t(_(label), nullptr, is_enabled_t::yes, is_hidden_t::no) {
    ChangeInformation(get_current_head_profile_name());
}

void MI_HEAD_CURRENT::click(IWindowMenu &window_menu) {
    (void)window_menu;
    // Refresh the information text
    ChangeInformation(get_current_head_profile_name());
}
