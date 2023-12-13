#include "gui.hpp"
#include "screen_menu.hpp"
#include "WindowMenuItems.hpp"
#include "MItem_menus.hpp"
#include "MItem_tools.hpp"
#include "menu_spin_config.hpp"
#include "screen_menu_snake_settings.hpp"

constexpr static const char *snake_settings_label = N_("SNAKE SETTINGS");

ScreenMenuSnakeSettings::ScreenMenuSnakeSettings()
    : ScreenMenuSnakeSettings__(_(snake_settings_label)) {
}

void ScreenMenuSnakeSettings::windowEvent(EventLock /*has private ctor*/, window_t *sender, GUI_event_t ev, void *param) {
    if (ev != GUI_event_t::CHILD_CLICK) {
        SuperWindowEvent(sender, ev, param);
        return;
    }
}

/*********************************************************************************/

ScreenMenuSnakeTuneSettings::ScreenMenuSnakeTuneSettings()
    : ScreenMenuSnakeTuneSettings__(_(snake_settings_label)) {
}

void ScreenMenuSnakeTuneSettings::windowEvent(EventLock /*has private ctor*/, window_t *sender, GUI_event_t ev, void *param) {
    if (ev != GUI_event_t::CHILD_CLICK) {
        SuperWindowEvent(sender, ev, param);
        return;
    }
}
