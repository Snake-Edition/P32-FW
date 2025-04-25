#include "bed_preheat.hpp"

#include "../../Marlin.h"
#include "../../module/temperature.h"
#include "../../gcode/gcode.h"
#include "../../lcd/ultralcd.h"
#include "../marlin_stubs/skippable_gcode.hpp"

#include <feature/print_status_message/print_status_message_guard.hpp>
#include <option/has_modularbed.h>

#if HAS_HEATED_BED

static constexpr int16_t minimal_preheat_temp = 60;
static constexpr int16_t minimal_temp_diff = 15;

void BedPreheat::update() {
    bool temp_near_target = thermalManager.degTargetBed() && std::abs(thermalManager.degBed() - thermalManager.degTargetBed()) < minimal_temp_diff;

    bool bedlets_changed = false;
    #if HAS_MODULARBED()
    if (thermalManager.getEnabledBedletMask() != last_enabled_bedlets) {
        last_enabled_bedlets = thermalManager.getEnabledBedletMask();
        bedlets_changed = true;
    }
    #endif

    if (temp_near_target && bedlets_changed == false) {
        if (heating_start_time.has_value() == false) {
            heating_start_time = millis();
        }
        can_preheat = true;
        if (remaining_preheat_time() == 0) {
            preheated = true;
        }
    } else {
        heating_start_time = std::nullopt;
        can_preheat = false;
        preheated = false;
    }
}

uint32_t BedPreheat::required_preheat_time() {
    if (thermalManager.degTargetBed() < minimal_preheat_temp) {
        return 0;
    }

    return std::max((180 + (thermalManager.degTargetBed() - 60) * (12 * 60 / 50)) * 1000, 0);
}

uint32_t BedPreheat::remaining_preheat_time() {
    if (preheated) {
        return 0;
    }

    const auto now = millis();
    const int32_t required = required_preheat_time();
    const int32_t elapsed = now - heating_start_time.value_or(now);
    return std::max((required - elapsed) / 1000, int32_t(0));
}

void BedPreheat::wait_for_preheat() {
    SkippableGCode::Guard skippable_operation;
    PrintStatusMessageGuard status_guard;

    while (can_preheat && !preheated && !skippable_operation.is_skip_requested()) {
        idle(true);

        // make sure we don't turn off the motors
        gcode.reset_stepper_timeout();

        status_guard.update<PrintStatusMessage::absorbing_heat>({ .current = (float)remaining_preheat_time(), .target = 0 });
    }
}

BedPreheat bed_preheat;

#endif
