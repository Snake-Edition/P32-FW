#pragma once
#include <stdint.h>

enum eEXTRUDER_TYPE : uint8_t {
    EXTRUDER_TYPE_PRUSA = 0,
    EXTRUDER_TYPE_BONDTECH = 1,
    EXTRUDER_TYPE_USER_USE_M92 = 2,
};
