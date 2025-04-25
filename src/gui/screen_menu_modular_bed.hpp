#pragma once

#include "WindowMenuItems.hpp"
#include <option/has_modular_bed.h>

static_assert(HAS_MODULAR_BED());

class MI_HEAT_ENTIRE_BED final : public WI_ICON_SWITCH_OFF_ON_t {
public:
    MI_HEAT_ENTIRE_BED();

private:
    virtual void OnChange(size_t old_index) override;
};
