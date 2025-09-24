#pragma once

#include "cmsis_os.h" // for osThreadId
#include <limits>
#include <time.h>

namespace marlin_server {

inline constexpr uint32_t TIME_TO_END_INVALID = std::numeric_limits<uint32_t>::max();
inline constexpr time_t TIMESTAMP_INVALID = std::numeric_limits<time_t>::max();

inline constexpr uint8_t CURRENT_TOOL = std::numeric_limits<uint8_t>::max();

extern osThreadId server_task; // task of marlin server

enum class KnobMove : uint8_t {
    NoMove,
    Up,
    Down,
};
} // namespace marlin_server
