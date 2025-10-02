/// \file

#pragma once

#include <marlin_server_types/client_response.hpp>
#include <utils/enum_array.hpp>
#include <utils/storage/enum_bitset.hpp>

enum class PhaseSafetyTimer : PhaseUnderlyingType {
    /// Blockingly resuming to original temperatures
    /// Passes the progress as float 0-100 via phase data
    resuming,

    /// Confirm abort of resuming/print
    abort_confirm,

    _cnt,
    _last = _cnt - 1
};

namespace ClientResponses {
extern constinit const EnumArray<PhaseSafetyTimer, PhaseResponses, PhaseSafetyTimer::_cnt> safety_timer_responses;
} // namespace ClientResponses

extern constinit const EnumBitset<PhaseSafetyTimer, PhaseSafetyTimer::_cnt> safety_timer_is_phase_attention;

constexpr inline ClientFSM client_fsm_from_phase(PhaseSafetyTimer) { return ClientFSM::SafetyTimer; }
