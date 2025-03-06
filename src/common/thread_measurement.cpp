/// @file
#include <common/thread_measurement.h>

#include <common/filament_sensors_handler.hpp>
#include <common/metric.h>
#include <common/tmc.h>
#include <freertos/timing.hpp>
#include <option/has_burst_stepping.h>
#include <option/has_phase_stepping.h>

#if HAS_PHASE_STEPPING()
    #include <feature/phase_stepping/phase_stepping.hpp>
#endif

METRIC_DEF(metric_tmc_sg_x, "tmc_sg_x", METRIC_VALUE_INTEGER, 10, METRIC_DISABLED);
METRIC_DEF(metric_tmc_sg_y, "tmc_sg_y", METRIC_VALUE_INTEGER, 10, METRIC_DISABLED);
METRIC_DEF(metric_tmc_sg_z, "tmc_sg_z", METRIC_VALUE_INTEGER, 10, METRIC_DISABLED);
METRIC_DEF(metric_tmc_sg_e, "tmc_sg_e", METRIC_VALUE_INTEGER, 10, METRIC_DISABLED);

static void sample_sg(metric_t *metric, uint8_t axis) {
    if (metric->enabled) {
        metric_record_integer(metric, tmc_sg_result(axis));
    }
}

static void sample_sg() {
#if HAS_PHASE_STEPPING() && !HAS_BURST_STEPPING()
    if (phase_stepping::any_axis_enabled()) {
        return;
    }
#endif
    sample_sg(&metric_tmc_sg_x, 0);
    sample_sg(&metric_tmc_sg_y, 1);
    sample_sg(&metric_tmc_sg_z, 2);
    sample_sg(&metric_tmc_sg_e, 3);
}

void StartMeasurementTask(void const *) {
    FSensors_instance().task_init();
    for (;;) {
        FSensors_instance().task_cycle();
        sample_sg();
        freertos::delay(50);
    }
}
