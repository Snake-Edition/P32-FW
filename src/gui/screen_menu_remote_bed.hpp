#pragma once

#include "WindowItemFormatableLabel.hpp"
#include <option/has_remote_bed.h>

static_assert(HAS_REMOTE_BED());

class MI_INFO_REMOTE_BED_MCU_TEMPERATURE : public MenuItemAutoUpdatingLabel<float> {
public:
    MI_INFO_REMOTE_BED_MCU_TEMPERATURE();
};
