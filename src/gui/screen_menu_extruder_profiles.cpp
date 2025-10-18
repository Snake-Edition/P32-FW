/**
 * @file screen_menu_extruder_profiles.cpp
 * @brief Implementation of extruder profiles menu screen
 */

#include "screen_menu_extruder_profiles.hpp"
#include "ScreenHandler.hpp"

ScreenMenuExtruderProfiles::ScreenMenuExtruderProfiles()
    : ScreenMenuExtruderProfiles__(_(label)) {
}

void ScreenMenuExtruderProfiles::windowEvent(window_t *sender, GUI_event_t event, void *param) {
    ScreenMenuExtruderProfiles__::windowEvent(sender, event, param);
}
