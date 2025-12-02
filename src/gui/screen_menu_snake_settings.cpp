#include "gui.hpp"
#include "screen_menu.hpp"
#include "WindowMenuItems.hpp"
#include "MItem_menus.hpp"
#include "MItem_tools.hpp"
#include "screen_menu_snake_settings.hpp"

constexpr static const char *snake_settings_label = N_("SNAKE SETTINGS");

ScreenMenuSnakeSettings::ScreenMenuSnakeSettings()
    : ScreenMenuSnakeSettings__(_(snake_settings_label)) {
}

void ScreenMenuSnakeSettings::windowEvent(window_t *, GUI_event_t, void *) {}
