#include "phase_stepping.hpp"
#include <option/has_phase_stepping_calibration.h>

namespace phase_stepping {

#if !HAS_PHASE_STEPPING_CALIBRATION()

// stubs that do nothing
static inline void calibration_move_cleanup(const AxisState &axis_state) {};
static inline void calibration_new_move(const AxisState &axis_state) {};
static inline correction_t calibration_phase_correction(const AxisState &axis_state,
    const CorrectedCurrentLut &lut, float current_position, int phase) {
    return lut.get_correction(phase);
}

#else

namespace internal {
    extern std::atomic<int> calibration_axis_active;
    extern std::atomic<int> calibration_axis_request;

    void calibration_new_move(const AxisState &axis_state);
    std::tuple<int, int, int> compute_calibration_tweak(float relative_position);
} // namespace internal

static inline bool calibration_active_on_axis(const AxisState &axis_state) {
    return internal::calibration_axis_active == axis_state.axis_index;
}

static inline void calibration_move_cleanup(const AxisState &axis_state) {
    if (calibration_active_on_axis(axis_state)) {
        internal::calibration_axis_active = -1;
        internal::calibration_axis_request = -1;
    }
}

static inline void calibration_new_move(const AxisState &axis_state) {
    // Make sweep active on current axis if the move is long enough
    if (internal::calibration_axis_request == axis_state.axis_index && axis_state.is_cruising) {
        internal::calibration_new_move(axis_state);
    }
}

static inline correction_t calibration_phase_correction(const AxisState &axis_state,
    const CorrectedCurrentLut &lut, float current_position, int phase) {
    if (calibration_active_on_axis(axis_state)) {
        float start_position = axis_state.current_target->initial_pos;
        float relative_position = current_position - start_position;
        auto [harmonic, pha, mag] = internal::compute_calibration_tweak(relative_position);
        return lut.get_correction_for_calibration(phase, harmonic, pha, mag);
    }

    return lut.get_correction(phase);
}

#endif // !HAS_PHASE_STEPPING_CALIBRATION()

} // namespace phase_stepping
