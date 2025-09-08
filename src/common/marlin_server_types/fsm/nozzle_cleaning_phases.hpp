/// \file

#pragma once

#include <marlin_server_types/client_response.hpp>

enum class PhaseNozzleCleaning : PhaseUnderlyingType {

    // Nozzle cleaning failed warning
    cleaning_failed,

    // On retry
    recommend_purge,

    // After purge
    remove_filament,

    _cnt,
    _last = _cnt - 1
};

namespace ClientResponses {

inline constexpr EnumArray<PhaseNozzleCleaning, PhaseResponses, PhaseNozzleCleaning::_cnt> nozzle_cleaning_responses {
    { PhaseNozzleCleaning::cleaning_failed, { Response::Retry, Response::Ignore, Response::Abort } },
    { PhaseNozzleCleaning::recommend_purge, { Response::Purge, Response::Ignore } },
    { PhaseNozzleCleaning::remove_filament, { Response::Done, Response::Abort } }
};

} // namespace ClientResponses

#if HAS_LOADCELL()

constexpr inline ClientFSM client_fsm_from_phase(PhaseNozzleCleaning) { return ClientFSM::NozzleCleaning; }

#endif
