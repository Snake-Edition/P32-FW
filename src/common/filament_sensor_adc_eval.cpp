#include "filament_sensor_adc_eval.hpp"

#include <cmath>

namespace FSensorADCEval {

FilamentSensorState evaluate_state(int32_t filtered_value, int32_t fs_ref_nins_value, int32_t fs_ref_ins_value, FilamentSensorState previous_state) {
    if (filtered_value == filtered_value_not_ready) {
        return FilamentSensorState::NotInitialized;
    }

    const auto is_calibrated = [](int32_t val) {
        return val >= lower_limit && val <= upper_limit;
    };

    // reference value not calibrated or out of sensible bounds
    if (!is_calibrated(fs_ref_ins_value) || !is_calibrated(fs_ref_nins_value)) {
        return FilamentSensorState::NotCalibrated;
    }

    // filtered value is out of sensible bounds
    if (filtered_value < lower_limit || filtered_value > upper_limit) {
        return FilamentSensorState::NotConnected;
    }

    const auto midpoint = (fs_ref_nins_value + fs_ref_ins_value) / 2;

    // Apply hysteresis if the filtered value is close to the midpoint
    if (
        // Cannot apply hysteresis if the previous state is not known
        (previous_state == FilamentSensorState::HasFilament || previous_state == FilamentSensorState::NoFilament)

        // We are splitting the range into thirds, but since the distance is from the midpoint, we actually have to divide by 6
        && std::abs(filtered_value - midpoint) < std::abs(fs_ref_nins_value - fs_ref_ins_value) / 6 //
    ) {
        return previous_state;
    }

    // Return whichever state the filtered value is closest to
    // Depending on the inserted magnet orientation, nins can be greater than ins - we need to account for that
    return (filtered_value > midpoint) == (fs_ref_ins_value > midpoint) ? FilamentSensorState::HasFilament : FilamentSensorState::NoFilament;
}

} // namespace FSensorADCEval
