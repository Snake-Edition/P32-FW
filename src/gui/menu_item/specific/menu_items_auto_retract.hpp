/// @file
#pragma once

#include "WindowMenuItems.hpp"

class MI_AUTO_RETRACT_ENABLE : public WI_ICON_SWITCH_OFF_ON_t {
public:
    MI_AUTO_RETRACT_ENABLE();
    void OnChange(size_t) final;
};
