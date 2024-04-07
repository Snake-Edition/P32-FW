#include "gui.hpp"
#include "screen_menu.hpp"
#include "WindowMenuItems.hpp"
#include "MItem_menus.hpp"
#include "MItem_tools.hpp"
#include "screen_menu_change_filament.hpp"
#include "marlin_client.hpp"
#include "marlin_server.hpp"

constexpr static const char *change_filament_label = N_("CHANGE FILAMENT");

ScreenMenuM600::ScreenMenuM600()
    : ScreenMenuChangeFilament__(_(change_filament_label)) {
}

void ScreenMenuM600::windowEvent(window_t *sender, GUI_event_t event, void *param) {
    switch (event) {
    case GUI_event_t::LOOP: {
        const auto current_command = marlin_client::get_command();
        Item<MI_M600>().set_enabled( //
            marlin_server::all_axes_homed()
            && marlin_server::all_axes_known()
            && (current_command != marlin_server::Cmd::G28)
            && (current_command != marlin_server::Cmd::G29)
            && (current_command != marlin_server::Cmd::M109)
            && (current_command != marlin_server::Cmd::M190) //
        );

        if (current_command == marlin_server::Cmd::M600) {
            // Once M600 is enqueued, it is no longer possible to enqueue another M600 from Tune menu
            // This resets the behaviour once M600 is executed
            Item<MI_M600>().resetEnqueued();
        }
    }

    default:
        break;
    }
    ScreenMenu::windowEvent(sender, event, param);
}
