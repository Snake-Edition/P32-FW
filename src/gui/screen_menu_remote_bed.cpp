#include "screen_menu_remote_bed.hpp"

#include "units.hpp"
#include <common/sensor_data.hpp>

MI_INFO_REMOTE_BED_MCU_TEMPERATURE::MI_INFO_REMOTE_BED_MCU_TEMPERATURE()
    : MenuItemAutoUpdatingLabel {
        _("Bed MCU Temp"),
        standard_print_format::temp_c,
        [](auto) -> float { return sensor_data().bedMCUTemperature.load(); },
    } {}
