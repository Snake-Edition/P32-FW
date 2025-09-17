#include "menu_items_auto_retract.hpp"

#include <config_store/store_instance.hpp>
#include <window_msgbox.hpp>

MI_AUTO_RETRACT_ENABLE::MI_AUTO_RETRACT_ENABLE()
    : WI_ICON_SWITCH_OFF_ON_t(config_store().auto_retract_enabled.get(), _("Auto Retract")) {}

void MI_AUTO_RETRACT_ENABLE::OnChange(size_t) {
    if (!value() && MsgBoxWarning(_("Auto Retract allows faster filament unloads and helps nozzle cleaning failures.\nAre you sure you want to disable it?"), Responses_YesNo, 1) != Response::Yes) {
        set_value(true);
        return;
    }

    config_store().auto_retract_enabled.set(value());
}
