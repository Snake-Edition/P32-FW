#include "accelerometer.h"
#include <optional>
#include "accelerometer_local.hpp"

#include <option/has_local_accelerometer.h>
#include <hwio_pindef.h>
#include <freertos/timing.hpp>

static_assert(HAS_LOCAL_ACCELEROMETER());

#include <lis2dh12_poller.hpp>

static std::optional<LIS2DH12Poller> g_local_accelerometer_poller;

PrusaAccelerometer::PrusaAccelerometer()
#if PRINTER_IS_PRUSA_MK3_5()
    : output_enabler { buddy::hw::fanPrintTach, buddy::hw::Pin::State::high, buddy::hw::OMode::pushPull, buddy::hw::OSpeed::high }
    , output_pin { output_enabler.pin() }
#endif
{
    if (g_local_accelerometer_poller.has_value()) {
        bsod("Multiple access to local accelerometer");
    }
#if PRINTER_IS_PRUSA_MK3_5()
    g_local_accelerometer_poller.emplace(&SPI_HANDLE_FOR(accelerometer), output_pin, &htim9);
#else
    g_local_accelerometer_poller.emplace(&SPI_HANDLE_FOR(accelerometer), buddy::hw::acellCs, &htim9);
#endif

    constexpr int RETRIES = 5;
    for (int i = 0; i < RETRIES; i++) {
        if (g_local_accelerometer_poller->setup_accelerometer()) {
            g_local_accelerometer_poller->start();
            break;
        }
        freertos::delay(10);
    }
    // We cannot throw exceptions, hence prevent object construction. However,
    // this is OK as all readings will fail with communication error.
}

PrusaAccelerometer::~PrusaAccelerometer() {
    g_local_accelerometer_poller->stop();
    g_local_accelerometer_poller.reset();
}

void PrusaAccelerometer::clear() {
    auto sample = g_local_accelerometer_poller->get_sample();
    while (sample.has_value()) {
        sample = g_local_accelerometer_poller->get_sample();
    }
    g_local_accelerometer_poller->clear_overflow();
}

PrusaAccelerometer::Error PrusaAccelerometer::get_error() const {
    if (!g_local_accelerometer_poller->hw_good()) {
        return PrusaAccelerometer::Error::communication;
    }
    if (g_local_accelerometer_poller->overflow_count() > 0) {
        return PrusaAccelerometer::Error::overflow_sensor;
    }
    return PrusaAccelerometer::Error::none;
}

float PrusaAccelerometer::get_sampling_rate() const {
    return g_local_accelerometer_poller->get_sampling_rate();
}

PrusaAccelerometer::GetSampleResult PrusaAccelerometer::get_sample(RawAcceleration &raw_acceleration) {
    if (!g_local_accelerometer_poller->hw_good() || g_local_accelerometer_poller->overflow_count() > 0) {
        return GetSampleResult::error;
    }

    auto sample = g_local_accelerometer_poller->get_sample();
    if (!sample.has_value()) {
        return GetSampleResult::buffer_empty;
    }
    auto [x, y, z] = *sample;
    raw_acceleration.val[0] = x;
    raw_acceleration.val[1] = y;
    raw_acceleration.val[2] = z;

    return GetSampleResult::ok;
}

void prusa_accelerometer_handle_polling() {
    g_local_accelerometer_poller->polling_routine();
}

void prusa_accelerometer_handle_spi_finish() {
    g_local_accelerometer_poller->spi_finish_routine();
}
