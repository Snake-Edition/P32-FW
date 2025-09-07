/**
 * @file screen_menu_print_head_profiles.cpp
 */

#include "screen_menu_print_head_profiles.hpp"
#include "ScreenHandler.hpp"
#include "config_store/store_instance.hpp"
#include "i18n.h"

ScreenMenuPrintHeadProfiles::ScreenMenuPrintHeadProfiles()
    : ScreenMenuPrintHeadProfiles__(_(label)) {}

void ScreenMenuPrintHeadProfiles::update_gui() {
    // No dynamic values to set currently; placeholder for future
}

void ScreenMenuPrintHeadProfiles::windowEvent(window_t *sender, GUI_event_t event, void *param) {
    ScreenMenuPrintHeadProfiles__::windowEvent(sender, event, param);
}
