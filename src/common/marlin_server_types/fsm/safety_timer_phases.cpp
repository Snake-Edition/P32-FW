#include "safety_timer_phases.hpp"

constinit const EnumArray<PhaseSafetyTimer, PhaseResponses, PhaseSafetyTimer::_cnt> ClientResponses::safety_timer_responses {
    { PhaseSafetyTimer::resuming, { Response::Abort } },
    { PhaseSafetyTimer::abort_confirm, { Response::No, Response::Abort } },
};

constinit const EnumBitset<PhaseSafetyTimer, PhaseSafetyTimer::_cnt> safety_timer_is_phase_attention {
    { PhaseSafetyTimer::resuming, false },
    { PhaseSafetyTimer::abort_confirm, true },
};
