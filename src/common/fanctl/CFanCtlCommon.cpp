
#include "CFanCtlCommon.hpp"
#include <fanctl.hpp>
#include "metric.h"
#include "Marlin/src/module/motion.h" // for active_extruder
#include <common/sensor_data.hpp>
#include <utils/utility_extensions.hpp>
#include <cmath>

bool CFanCtlCommon::is_fan_ok() const {
    if (selftest_mode || get_pwm() == 0) {
        return true;
    }

    const auto state = get_state();
    return get_rpm_is_ok() || (state != running && state != error_running && state != error_starting);
}

void record_fanctl_metrics() {
    METRIC_DEF(metric, "fan", METRIC_VALUE_CUSTOM, 0, METRIC_ENABLED);
    METRIC_DEF(fan_print, "print_fan_act", METRIC_VALUE_INTEGER, 1000, METRIC_DISABLED);
    METRIC_DEF(fan_hbr, "hbr_fan_act", METRIC_VALUE_INTEGER, 1000, METRIC_DISABLED);
#if XL_ENCLOSURE_SUPPORT() // XLBOARD has additional enclosure fan
    METRIC_DEF(fan_enclosure, "hbr_fan_enc", METRIC_VALUE_INTEGER, 1000, METRIC_DISABLED);
#endif
    static uint32_t last_update = 0;
    static constexpr uint32_t UPDATE_PERIOD = 987;

    auto record = [](const auto &fanctl, const char *fan_name) {
        const int8_t state = std::to_underlying(fanctl.get_state());
        const float pwm = fanctl.get_pwm() * 100.f / fanctl.get_max_pwm();
        const float measured = fanctl.get_actual_rpm() * 100.f / fanctl.get_max_rpm();

        metric_record_custom(&metric, ",fan=%s state=%i,pwm=%i,measured=%i",
            fan_name, state, static_cast<int>(std::lround(pwm)), static_cast<int>(std::lround(measured)));
    };

    if (HAL_GetTick() - last_update > UPDATE_PERIOD) {
        record(Fans::print(active_extruder), "print");
        metric_record_integer(&fan_print, Fans::print(active_extruder).get_actual_rpm());
        record(Fans::heat_break(active_extruder), "heatbreak");
        metric_record_integer(&fan_hbr, Fans::heat_break(active_extruder).get_actual_rpm());
#if XL_ENCLOSURE_SUPPORT() // XLBOARD has additional enclosure fan
        record(Fans::enclosure(), "enclosure");
        metric_record_integer(&fan_enclosure, Fans::enclosure().get_actual_rpm());
#endif
        last_update = HAL_GetTick();
    }
}
