#include "accelerometer.h"

#include "toolchanger.h"
#include "accelerometer_utils.h"
#include <puppies/Dwarf.hpp>
#include "bsod.h"
#include "../../core/serial.h"
#include <mutex>
#include <option/has_remote_accelerometer.h>

static_assert(HAS_REMOTE_ACCELEROMETER());

freertos::Mutex PrusaAccelerometer::s_buffer_mutex;
PrusaAccelerometer::SampleBuffer *PrusaAccelerometer::s_sample_buffer = nullptr;
float PrusaAccelerometer::m_sampling_rate = 0;

/**
 * If this is the first instance of PrusaAccelerometer
 * assign address of our m_sample_buffer to s_sample_buffer
 * and enable active Dwarf accelerometer.
 * Do nothing otherwise.
 */
PrusaAccelerometer::PrusaAccelerometer() {
    {
        std::lock_guard lock(s_buffer_mutex);
        if (s_sample_buffer) {
            m_sample_buffer.error.set(Error::busy);
            return;
        }

        s_sample_buffer = &m_sample_buffer;
    }

    buddy::puppies::Dwarf *dwarf = prusa_toolchanger.get_marlin_picked_tool();
    if (!dwarf) {
        std::lock_guard lock(s_buffer_mutex);
        m_sample_buffer.error.set(Error::no_active_tool);
        return;
    }

    if (!dwarf->set_accelerometer(true)) {
        std::lock_guard lock(s_buffer_mutex);
        m_sample_buffer.error.set(Error::communication);
        return;
    }
}
/**
 * If there is address of our m_sample_buffer in s_sample_buffer
 * remove it and disable Dwarf accelerometer.
 * Do nothing otherwise.
 */
PrusaAccelerometer::~PrusaAccelerometer() {
    {
        std::lock_guard lock(s_buffer_mutex);

        if (&m_sample_buffer != s_sample_buffer) {
            return;
        }

        s_sample_buffer = nullptr;
    }
    /// @note We need to exit mutex before calling Dwarf methods to prevent deadlock.

    switch (m_sample_buffer.error.get()) {
    case Error::none:
    case Error::communication:
    case Error::overflow_sensor:
    case Error::overflow_buddy:
    case Error::overflow_dwarf:
    case Error::overflow_possible: {
        buddy::puppies::Dwarf *dwarf = prusa_toolchanger.get_marlin_picked_tool();
        if (!dwarf) {
            break;
        }
        if (!dwarf->set_accelerometer(false)) {
            SERIAL_ERROR_MSG("Failed to disable accelerometer, communication error");
        }
        break;
    }
    case Error::no_active_tool:
        break;

    case Error::busy:
    case Error::_cnt:
        bsod("Unexpected");
    }
}

void PrusaAccelerometer::clear() {
    // todo wait for for so many samples that it is assured
    // that even if all buffers were full we went through
    // all samples and reflect possible delay in steps_to_do_max
    std::lock_guard lock(s_buffer_mutex);
    m_sample_buffer.buffer.clear();
    m_sample_buffer.error.clear_overflow();
}

PrusaAccelerometer::GetSampleResult PrusaAccelerometer::get_sample(Acceleration &acceleration) {
    std::lock_guard lock(s_buffer_mutex);
    if (get_error() != Error::none) {
        return GetSampleResult::error;
    }

    common::puppies::fifo::AccelerometerXyzSample sample;
    if (!m_sample_buffer.buffer.try_get(sample)) {
        return GetSampleResult::buffer_empty;
    }

    AccelerometerUtils::SampleStatus sample_status;
    acceleration = AccelerometerUtils::unpack_sample(sample_status, sample);

    if (sample_status.buffer_overflow) {
        m_sample_buffer.error.set(Error::overflow_dwarf);
        return GetSampleResult::error;
    }

    if (sample_status.sample_overrun) {
        m_sample_buffer.error.set(Error::overflow_sensor);
        return GetSampleResult::error;
    }

    return GetSampleResult::ok;
}

float PrusaAccelerometer::get_sampling_rate() const {
    return m_sampling_rate;
}

PrusaAccelerometer::Error PrusaAccelerometer::get_error() const {
    return m_sample_buffer.error.get();
}

void PrusaAccelerometer::put_sample(common::puppies::fifo::AccelerometerXyzSample sample) {
    std::lock_guard lock(s_buffer_mutex);
    if (s_sample_buffer) {
        if (!s_sample_buffer->buffer.try_put(sample)) {
            s_sample_buffer->error.set(Error::overflow_buddy);
        }
    }
}

void PrusaAccelerometer::set_possible_overflow() {
    std::lock_guard lock(s_buffer_mutex);
    if (s_sample_buffer) {
        s_sample_buffer->error.set(Error::overflow_possible);
    }
}

void PrusaAccelerometer::set_rate(float rate) {
    m_sampling_rate = rate;
}
