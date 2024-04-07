#include "gui.hpp"
#include "screen_menu.hpp"
#include "WindowMenuItems.hpp"
#include "MItem_menus.hpp"
#include "MItem_tools.hpp"
#include "menu_spin_config.hpp"
#include "screen_menu_change_filament.hpp"

constexpr static const char *change_filament_label = N_("CHANGE FILAMENT");

ScreenMenuM600::ScreenMenuM600()
    : ScreenMenuChangeFilament__(_(change_filament_label)) {
}

void ScreenMenuM600::windowEvent(EventLock /*has private ctor*/, window_t *sender, GUI_event_t ev, void *param) {
    switch (ev) {
    case GUI_event_t::LOOP:
        if (marlin_server::all_axes_homed()
            && marlin_server::all_axes_known()
            && (marlin_client::get_command() != marlin_server::Cmd::G28)
            && (marlin_client::get_command() != marlin_server::Cmd::G29)
            && (marlin_client::get_command() != marlin_server::Cmd::M109)
            && (marlin_client::get_command() != marlin_server::Cmd::M190)) {
            Item<MI_M600>().Enable();
        } else {
            Item<MI_M600>().Disable();
        }
        break;
    default:
        break;
    }

    if (ev != GUI_event_t::CHILD_CLICK) {
        SuperWindowEvent(sender, ev, param);
        return;
    }
}
