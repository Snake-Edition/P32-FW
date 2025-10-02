/// \file
#pragma once

#include <marlin_server_types/client_response.hpp>

// Frequency has 0.5f step and we need to send this through 4B
struct belt_tensions {
    uint16_t upper_belt_tension = 0; /// Tension is passed multiplied by 2
    uint16_t lower_belt_tension = 0; /// Tension is passed multiplied by 2

    belt_tensions() = default;

    belt_tensions(float upper, float lower) {
        set_upper(upper);
        set_lower(lower);
    }

    void set_upper(float upper_belt) { upper_belt_tension = static_cast<uint16_t>(upper_belt * 2); }
    float get_upper() const { return static_cast<float>(upper_belt_tension) / 2.0f; }
    void set_lower(float lower_belt) { lower_belt_tension = static_cast<uint16_t>(lower_belt * 2); }
    float get_lower() const { return static_cast<float>(lower_belt_tension) / 2.0f; }
};

struct screw_revs {
    int8_t turn_eights;
};

enum class PhaseManualBeltTuning : PhaseUnderlyingType {
    /// Introduction to belt tuning calibration, QR, prerequisites
    intro,

    /// Checking if X-axis gantry is correctly lined up
    check_x_gantry,

    /// Wait for the printer to home and move to wizard position
    homing_wait,

    /// Prepare for measuring actual frequencies, QR, prerequisites
    intro_measure,

    /// Measuring actual upper belt frequency, Knob
    measure_upper_belt,

    /// Measuring actual lower belt frequency, Knob
    measure_lower_belt,

    /// Show results of both belt measurements in Newtons
    show_tension,

    /// Alignment issue detected, x-gantry is most likely bended
    alignment_issue,

    /// User adjusts the tensioners with calculated allen key turns
    adjust_tensioners,

    /// Calibration successfully completed
    finished,

    _cnt,
    _last = _cnt - 1
};

namespace ClientResponses {

inline constexpr EnumArray<PhaseManualBeltTuning, PhaseResponses, PhaseManualBeltTuning::_cnt> manual_belt_tuning_responses {
    { PhaseManualBeltTuning::intro, { Response::Continue, Response::Abort } },
    { PhaseManualBeltTuning::check_x_gantry, { Response::Continue, Response::Abort } },
    { PhaseManualBeltTuning::homing_wait, {} },
    { PhaseManualBeltTuning::intro_measure, { Response::Continue, Response::Abort } },
    { PhaseManualBeltTuning::measure_upper_belt, { Response::Done, Response::Abort } },
    { PhaseManualBeltTuning::measure_lower_belt, { Response::Done, Response::Abort } },
    { PhaseManualBeltTuning::show_tension, { Response::Continue, Response::Abort } },
    { PhaseManualBeltTuning::alignment_issue, { Response::Retry, Response::Abort } },
    { PhaseManualBeltTuning::adjust_tensioners, { Response::Continue, Response::Adjust, Response::Abort } },
    { PhaseManualBeltTuning::finished, { Response::Finish } },
};

} // namespace ClientResponses

constexpr inline ClientFSM client_fsm_from_phase(PhaseManualBeltTuning) { return ClientFSM::ManualBeltTuning; }
