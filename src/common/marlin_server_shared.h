#pragma once

#include "cmsis_os.h" // for osThreadId
#include <limits>
#include <time.h>
#include <option/has_selftest.h>
#include <option/has_cancel_object.h>

namespace marlin_server {

inline constexpr uint32_t TIME_TO_END_INVALID = std::numeric_limits<uint32_t>::max();
inline constexpr time_t TIMESTAMP_INVALID = std::numeric_limits<time_t>::max();

inline constexpr uint8_t CURRENT_TOOL = std::numeric_limits<uint8_t>::max();

extern osThreadId server_task; // task of marlin server

enum class RequestFlag : uint8_t {
    PrintReady,
    PrintAbort,
    PrintPause,
    PrintResume,
    TryRecoverFromMediaError,
    PrintExit,
    KnobMove,
    KnobClick,
    GuiCantPrint,
#if HAS_SELFTEST()
    TestAbort,
#endif
#if HAS_CANCEL_OBJECT()
    CancelCurrentObject,
#endif
    _cnt
};

} // namespace marlin_server
