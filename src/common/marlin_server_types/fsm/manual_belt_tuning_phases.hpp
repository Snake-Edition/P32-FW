/// \file
#pragma once

#include <marlin_server_types/client_response.hpp>

struct belt_tensions {
    uint8_t upper_belt_tension = 0;
    uint8_t lower_belt_tension = 0;
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

    /// User adjusts the tensioners with calculated allen key turns
    adjust_tensioners,

    /// Prepare to recheck belt tension with target frequency
    intro_recheck_target_freq,

    /// Recheck upper belt with target frequency
    recheck_upper_belt,

    /// Recheck lower belt with target frequency
    recheck_lower_belt,

    _cnt,
    _last = _cnt - 1
};

namespace ClientResponses {

inline constexpr EnumArray<PhaseManualBeltTuning, PhaseResponses, PhaseManualBeltTuning::_cnt> belt_tuning_responses {
    { PhaseManualBeltTuning::intro, { Response::Continue, Response::Abort } },
    { PhaseManualBeltTuning::check_x_gantry, { Response::Continue, Response::Abort } },
    { PhaseManualBeltTuning::homing_wait, {} },
    { PhaseManualBeltTuning::intro_measure, { Response::Continue, Response::Abort } },
    { PhaseManualBeltTuning::measure_upper_belt, { Response::Done, Response::Abort } },
    { PhaseManualBeltTuning::measure_lower_belt, { Response::Done, Response::Abort } },
    { PhaseManualBeltTuning::show_tension, { Response::Continue, Response::Abort } },
    { PhaseManualBeltTuning::adjust_tensioners, { Response::Continue, Response::Abort } },
    { PhaseManualBeltTuning::intro_recheck_target_freq, { Response::Continue, Response::Abort } },
    { PhaseManualBeltTuning::recheck_upper_belt, { Response::Continue, Response::Adjust } },
    { PhaseManualBeltTuning::recheck_lower_belt, { Response::Continue, Response::Adjust } },
};

} // namespace ClientResponses

constexpr inline ClientFSM client_fsm_from_phase(PhaseManualBeltTuning) { return ClientFSM::BeltTuning; }
