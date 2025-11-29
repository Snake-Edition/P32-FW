#include "accelerometer_utils.h"

#include <option/has_remote_accelerometer.h>

static_assert(HAS_REMOTE_ACCELEROMETER());

/**
 * Unpack 10bit samples into 16bit sample and swap X and Z axis to compensate for Dwarf orientation
 */
PrusaAccelerometer::RawAcceleration AccelerometerUtils::unpack_sample(AccelerometerUtils::SampleStatus &sampleStatus, common::puppies::fifo::AccelerometerXyzSample sample) {
    constexpr int16_t top_10_bits = 0b1111'1111'1100'0000u;

    PrusaAccelerometer::RawAcceleration accelerometer_sample;
    accelerometer_sample.val[2] = (sample << x_right_shift) & top_10_bits;
    accelerometer_sample.val[1] = (sample >> y_left_shift) & top_10_bits;
    accelerometer_sample.val[0] = (sample >> z_left_shift) & top_10_bits;
    sampleStatus.buffer_overflow = sample & buffer_overflow_mask;
    sampleStatus.sample_overrun = sample & sample_overrun_mask;
    return accelerometer_sample;
}
