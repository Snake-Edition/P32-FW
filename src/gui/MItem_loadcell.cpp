#include "MItem_loadcell.hpp"

#include <sensor_data.hpp>

/*****************************************************************************/
// MI_INFO_LOADCELL
MI_INFO_LOADCELL::MI_INFO_LOADCELL()
    : MenuItemAutoUpdatingLabel(
        _("Loadcell Value"), "%.1f",
        [](auto) { return sensor_data().loadCell.load(); } //
    ) {}
