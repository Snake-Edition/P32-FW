/**
 * @file screen_menu_extruder_profiles.hpp
 * @brief Menu screen for selecting extruder profiles
 */

#pragma once

#include "screen_menu.hpp"
#include "WindowMenuItems.hpp"
#include "MItem_extruder_profiles.hpp"
#include "MItem_menus.hpp"

using ScreenMenuExtruderProfiles__ = ScreenMenu<GuiDefaults::MenuFooter,
    MI_RETURN,
    MI_CURRENT_PROFILE,
    MI_PROFILE_STANDARD,
    MI_PROFILE_BONDTECH,
    MI_PROFILE_BINUS_DUALDRIVE,
    MI_PROFILE_CUSTOM>;

class ScreenMenuExtruderProfiles : public ScreenMenuExtruderProfiles__ {
public:
    constexpr static const char *label = N_("EXTRUDER PROFILES");
    ScreenMenuExtruderProfiles();

private:
    virtual void windowEvent(window_t *sender, GUI_event_t event, void *param) override;
};
