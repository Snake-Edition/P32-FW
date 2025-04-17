#pragma once

#include <option/has_dwarf.h>
#include <option/has_modularbed.h>
#include <option/has_toolchanger.h>
#include <option/has_loadcell.h>
#include <option/has_selftest.h>
#include <option/has_phase_stepping.h>
#include <option/has_coldpull.h>
#include <option/has_input_shaper_calibration.h>
#include <option/has_belt_tuning.h>
#include <option/has_side_fsensor.h>
#include <option/has_emergency_stop.h>
#include <option/xl_enclosure_support.h>
#include <option/has_chamber_api.h>
#include <option/has_uneven_bed_prompt.h>
#include <option/has_door_sensor_calibration.h>

#include <inc/MarlinConfigPre.h>

#include <stdint.h>
#include <utils/utility_extensions.hpp>

#ifdef __cplusplus
// C++ checks enum classes

// Client finite state machines
// bound to src/client_response.hpp
enum class ClientFSM : uint8_t {
    Serial_printing,
    Load_unload,
    Preheat,
    #if HAS_SELFTEST()
    Selftest,
    #endif
    FansSelftest,
    NetworkSetup,
    Printing, // not a dialog
    #if ENABLED(CRASH_RECOVERY)
    CrashRecovery,
    #endif
    QuickPause,
    Warning,
    PrintPreview,
    #if HAS_COLDPULL()
    ColdPull,
    #endif
    #if HAS_PHASE_STEPPING()
    PhaseStepping,
    #endif
    #if HAS_INPUT_SHAPER_CALIBRATION()
    InputShaperCalibration,
    #endif
    #if HAS_BELT_TUNING()
    BeltTuning,
    #endif
    #if HAS_DOOR_SENSOR_CALIBRATION()
    DoorSensorCalibration,
    #endif
    Wait, ///< FSM that only blocks the screen with a "please wait" text
    _none, // cannot be created, must have same index as _count
    _count = _none
};

// We have only 5 bits for it in the serialization of data sent between server and client
static_assert(ftrstd::to_underlying(ClientFSM::_count) < 32);

enum class LoadUnloadMode : uint8_t {
    Change,
    Load,
    Unload,
    Purge,
    FilamentStuck,
    Test,
    Cut, // MMU
    Eject, // MMU
};

enum class PreheatMode : uint8_t {
    None,
    Load,
    Unload,
    Purge,
    Change_phase1, // do unload, call Change_phase2 after load finishes
    Change_phase2, // do load, meant to be used recursively in Change_phase1
    Unload_askUnloaded,
    Autoload,
    _last = Autoload
};

enum class RetAndCool_t : uint8_t {
    Neither = 0b00,
    Cooldown = 0b01,
    Return = 0b10,
    Both = 0b11,
    last_ = Both
};

using message_cb_t = void (*)(char *);
#else // !__cplusplus
// C
typedef void (*message_cb_t)(const char *);
typedef void (*warning_cb_t)(uint32_t);
#endif //__cplusplus
