/**
 * @file MItem_extruder_profiles.cpp
 * @brief Implementation of extruder profile menu items
 */

#include "MItem_extruder_profiles.hpp"
#include "screen_menu_extruder_profiles.hpp"
#include "screen_menu_experimental_settings.hpp"
#include "ScreenHandler.hpp"
#include <ScreenFactory.hpp>
#include "marlin_client.hpp"
#include "config_store/store_instance.hpp"
#include <window_msgbox.hpp>
#include "persistent_stores/store_instances/config_store/store_c_api.h"
#include "i18n.h"
#include "lang/string_view_utf8.hpp"

// Extruder profile database
const ExtruderProfileData extruder_profiles[] = {
    // Standard Prusa extruder
    {
        .name = "Prusa",
        .steps_per_unit_e = 280.0f, // Standard Prusa MINI/MK3 E steps
        .reverse_direction = false, // Normal direction
        .max_feedrate_e = 120.0f, // mm/s
        .max_acceleration_e = 5000.0f, // mm/s²
        .max_jerk_e = 4.5f, // mm/s
        .retract_length = 3.2f, // mm
        .retract_speed = 70.0f // mm/s
    },
    // BondTech extruder
    {
        .name = "BondTech",
        .steps_per_unit_e = 415.0f, // BondTech BMG typical steps
        .reverse_direction = true, // Reversed direction (as per instructions)
        .max_feedrate_e = 120.0f, // mm/s
        .max_acceleration_e = 10000.0f, // Higher acceleration due to gearing
        .max_jerk_e = 4.5f, // mm/s
        .retract_length = 0.8f, // Shorter retraction due to gearing
        .retract_speed = 60.0f // mm/s
    },
    // Binus Dualdrive
    // https://www.printables.com/model/946290-dual-gear-drive-extruder-for-prusa-minimini#bom-bill-of-materials-
    {
        .name = "Binus Dualdrive",
        .steps_per_unit_e = 140.0f,
        .reverse_direction = false, // Normal direction
        .max_feedrate_e = 120.0f, // mm/s
        .max_acceleration_e = 5000.0f, // mm/s²
        .max_jerk_e = 4.5f, // mm/s
        .retract_length = 3.2f, // mm
        .retract_speed = 70.0f // mm/s
    },
};

const size_t extruder_profiles_count = sizeof(extruder_profiles) / sizeof(extruder_profiles[0]);

// Helper function to apply profile settings
static void apply_extruder_profile(ExtruderProfile profile_type) {
    if (profile_type >= static_cast<ExtruderProfile>(extruder_profiles_count)) {
        return;
    }

    const ExtruderProfileData &profile = extruder_profiles[static_cast<size_t>(profile_type)];

    // Show confirmation dialog
    if (MsgBoxQuestion(_("Apply extruder profile?"), Responses_YesNo) != Response::Yes) {
        return;
    }

    // Apply steps per unit to experimental settings
    if (profile.steps_per_unit_e > 0) {
        config_store().extruder_profile.set(static_cast<uint8_t>(profile_type));

        // Steps
        set_steps_per_unit_e(profile.steps_per_unit_e);

        // Direction
        if (profile.reverse_direction) {
            set_wrong_direction_e();
        } else {
            set_PRUSA_direction_e();
        }

        // Special: for BondTech also set Z axis length to 190 (per instructions)
        if (profile_type == ExtruderProfile::BondTech) {
            set_z_max_pos_mm(190.0f);
        }

        MsgBoxInfo(_("Profile applied. Save settings in Experimental menu."), Responses_Ok);
    }
}

// Helper function to get current profile name
const char *MI_CURRENT_PROFILE::get_current_profile_name() {
    uint8_t current_profile = config_store().extruder_profile.get();
    if (current_profile < extruder_profiles_count) {
        return extruder_profiles[current_profile].name;
    }
    return "Unknown";
}

// Menu item implementations
MI_EXTRUDER_PROFILES::MI_EXTRUDER_PROFILES()
    : IWindowMenuItem(_(label), nullptr, is_enabled_t::yes, is_hidden_t::no) {
}

// Provide an out-of-line destructor to anchor the vtable in this TU
MI_EXTRUDER_PROFILES::~MI_EXTRUDER_PROFILES() = default;

void MI_EXTRUDER_PROFILES::click(IWindowMenu &window_menu) {
    (void)window_menu;
    Screens::Access()->Open(ScreenFactory::Screen<ScreenMenuExtruderProfiles>);
}

MI_PROFILE_STANDARD::MI_PROFILE_STANDARD()
    : IWindowMenuItem(_(label), nullptr, is_enabled_t::yes, is_hidden_t::no) {
}

void MI_PROFILE_STANDARD::click(IWindowMenu &window_menu) {
    (void)window_menu;
    apply_extruder_profile(ExtruderProfile::Standard);
}

MI_PROFILE_BONDTECH::MI_PROFILE_BONDTECH()
    : IWindowMenuItem(_(label), nullptr, is_enabled_t::yes, is_hidden_t::no) {
}

void MI_PROFILE_BONDTECH::click(IWindowMenu &window_menu) {
    (void)window_menu;
    apply_extruder_profile(ExtruderProfile::BondTech);
}

MI_PROFILE_BINUS_DUALDRIVE::MI_PROFILE_BINUS_DUALDRIVE()
    : IWindowMenuItem(_(label), nullptr, is_enabled_t::yes, is_hidden_t::no) {
}

void MI_PROFILE_BINUS_DUALDRIVE::click(IWindowMenu &window_menu) {
    (void)window_menu;
    apply_extruder_profile(ExtruderProfile::Binus_Dualdrive);
}

MI_PROFILE_CUSTOM::MI_PROFILE_CUSTOM()
    : IWindowMenuItem(_(label), nullptr, is_enabled_t::yes, is_hidden_t::no) {
}

// Provide an out-of-line destructor to anchor the vtable in this TU
MI_PROFILE_CUSTOM::~MI_PROFILE_CUSTOM() = default;

void MI_PROFILE_CUSTOM::click(IWindowMenu &window_menu) {
    (void)window_menu;
    // Open experimental settings directly for custom configuration
    Screens::Access()->Open(ScreenFactory::Screen<ScreenMenuExperimentalSettings>);
}

MI_CURRENT_PROFILE::MI_CURRENT_PROFILE()
    : WI_INFO_t(_(label), nullptr, is_enabled_t::yes, is_hidden_t::no) {
    ChangeInformation(get_current_profile_name());
}

void MI_CURRENT_PROFILE::click(IWindowMenu &window_menu) {
    (void)window_menu;
    // Refresh the displayed profile name
    ChangeInformation(get_current_profile_name());
}
