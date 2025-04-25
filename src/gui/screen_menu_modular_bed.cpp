#include "screen_menu_modular_bed.hpp"

#include <common/marlin_client.hpp>
#include <config_store/store_instance.hpp>

MI_HEAT_ENTIRE_BED::MI_HEAT_ENTIRE_BED()
    : WI_ICON_SWITCH_OFF_ON_t {
        config_store().heat_entire_bed.get(),
        _("Heat Entire Bed"),
    } {}

void MI_HEAT_ENTIRE_BED::OnChange(size_t) {
    config_store().heat_entire_bed.set(value());
    if (value()) {
        marlin_client::gcode("M556 A"); // enable all bedlets now
    }
}
