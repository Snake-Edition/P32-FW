#pragma once

#include <option/has_remote_bed.h>

static_assert(HAS_REMOTE_BED());

namespace remote_bed {

float get_heater_current();

uint16_t get_mcu_temperature();

void safe_state();

} // namespace remote_bed
