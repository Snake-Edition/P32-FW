/**
 * @file client_response.hpp
 * @brief every phase in dialog can have some buttons
 * buttons are generalized on this level as responses
 * because non GUI/WUI client can also use them
 * bound to ClientFSM in src/common/client_fsm_types.h
 */

#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include <span>
#include <utility>

#include "client_fsm_types.h"
#include "general_response.hpp"
#include "printers.h"
#include <option/filament_sensor.h>
#include <option/has_attachable_accelerometer.h>
#include <option/has_belt_tuning.h>
#include <option/has_coldpull.h>
#include <option/has_emergency_stop.h>
#include <option/has_gears_calibration.h>
#include <option/has_input_shaper_calibration.h>
#include <option/has_loadcell.h>
#include <option/has_mmu2.h>
#include <option/has_nfc.h>
#include <option/has_phase_stepping.h>
#include <option/has_selftest.h>
#include <option/has_toolchanger.h>
#include <option/xl_enclosure_support.h>
#include <option/has_chamber_api.h>
#include <option/has_xbuddy_extension.h>
#include <option/has_uneven_bed_prompt.h>
#include <option/has_door_sensor_calibration.h>
#include <common/hotend_type.hpp>
#include <option/has_auto_retract.h>
#include <device/board.h>

/// number of bits used to encode response
// TODO: Make 2 everywhere: BFW-6028
#if HAS_MMU2() || HAS_TOOLCHANGER()
    #define RESPONSE_BITS 4
#else
    #define RESPONSE_BITS 2
#endif

/// maximum number of responses in one phase
#define MAX_RESPONSES (1 << RESPONSE_BITS)

using PhaseResponses = std::array<Response, MAX_RESPONSES>;
static constexpr PhaseResponses empty_phase_responses = {};

// count enum class members (if "_last" is defined)
template <class T>
constexpr size_t CountPhases() {
    return static_cast<size_t>(T::_last) + 1;
}
// use this when creating an event
// encodes enum to position in phase
template <class T>
constexpr uint8_t GetPhaseIndex(T phase) {
    return static_cast<size_t>(phase);
}

template <class T>
constexpr T GetEnumFromPhaseIndex(size_t index) {
    assert(index < CountPhases<T>());
    return static_cast<T>(index);
}

using PhaseUnderlyingType = uint8_t;

struct FSMAndPhase {

public:
    constexpr FSMAndPhase(ClientFSM fsm, PhaseUnderlyingType phase)
        : fsm(fsm)
        , phase(phase) {
    }

    template <typename Phase>
    constexpr FSMAndPhase(Phase phase)
        : fsm(client_fsm_from_phase(phase))
        , phase(std::to_underlying(phase)) {
    }

    constexpr FSMAndPhase(const FSMAndPhase &) = default;

    constexpr bool operator==(const FSMAndPhase &) const = default;
    constexpr bool operator!=(const FSMAndPhase &) const = default;

public:
    ClientFSM fsm;
    PhaseUnderlyingType phase;
};

enum class PhaseWait : PhaseUnderlyingType {
    generic,
    homing,
    _cnt,
    _last = _cnt - 1,
};
constexpr inline ClientFSM client_fsm_from_phase(PhaseWait) { return ClientFSM::Wait; }

// define enum classes for responses here
// and YES phase can have 0 responses
// every enum must have "_last"
// EVERY response shall have a unique ID (so every button in GUI is unique)
enum class PhasesLoadUnload : PhaseUnderlyingType {
    initial,
    ChangingTool,
    Parking_stoppable,
    Parking_unstoppable,
    WaitingTemp_stoppable,
    WaitingTemp_unstoppable,
    PreparingToRam_stoppable,
    PreparingToRam_unstoppable,
    Ramming_stoppable,
    Ramming_unstoppable,
    Unloading_stoppable,
    Unloading_unstoppable,
    RemoveFilament,
    IsFilamentUnloaded,
    FilamentNotInFS,
    ManualUnload_continuable,
    ManualUnload_uncontinuable,
    UserPush_stoppable,
    UserPush_unstoppable,
    MakeSureInserted_stoppable,
    MakeSureInserted_unstoppable,
    Inserting_stoppable,
    Inserting_unstoppable,
    IsFilamentInGear,
    Ejecting_stoppable,
    Ejecting_unstoppable,
    Loading_stoppable,
    Loading_unstoppable,
    Purging_stoppable,
    Purging_unstoppable,
    IsColor,
    IsColorPurge,
    Unparking,

#if HAS_LOADCELL()
    FilamentStuck,
#endif

#if HAS_AUTO_RETRACT()
    AutoRetracting,
#endif

#if HAS_MMU2()
    // MMU-specific dialogs
    LoadFilamentIntoMMU,
    // internal states of the MMU
    MMU_EngagingIdler,
    MMU_DisengagingIdler,
    MMU_UnloadingToFinda,
    MMU_UnloadingToPulley,
    MMU_FeedingToFinda,
    MMU_FeedingToBondtech,
    MMU_FeedingToNozzle,
    MMU_AvoidingGrind,
    MMU_FinishingMoves,
    MMU_ERRDisengagingIdler,
    MMU_ERREngagingIdler,
    MMU_ERRWaitingForUser,
    MMU_ERRInternal,
    MMU_ERRHelpingFilament,
    MMU_ERRTMCFailed,
    MMU_UnloadingFilament,
    MMU_LoadingFilament,
    MMU_SelectingFilamentSlot,
    MMU_PreparingBlade,
    MMU_PushingFilament,
    MMU_PerformingCut,
    MMU_ReturningSelector,
    MMU_ParkingSelector,
    MMU_EjectingFilament,
    MMU_RetractingFromFinda,
    MMU_Homing,
    MMU_MovingSelector,
    MMU_FeedingToFSensor,
    MMU_HWTestBegin,
    MMU_HWTestIdler,
    MMU_HWTestSelector,
    MMU_HWTestPulley,
    MMU_HWTestCleanup,
    MMU_HWTestExec,
    MMU_HWTestDisplay,
    MMU_ErrHwTestFailed,
#endif

    _cnt,
    _last = _cnt - 1
};
constexpr inline ClientFSM client_fsm_from_phase(PhasesLoadUnload) { return ClientFSM::Load_unload; }

enum class PhasesPreheat : PhaseUnderlyingType {
    initial,
    UserTempSelection,
    _last = UserTempSelection
};
constexpr inline ClientFSM client_fsm_from_phase(PhasesPreheat) { return ClientFSM::Preheat; }

enum class PhasesPrintPreview : PhaseUnderlyingType {
    loading,
    download_wait,
    main_dialog,
    unfinished_selftest,
    new_firmware_available,
    wrong_printer,
    wrong_printer_abort,
    filament_not_inserted,
#if HAS_MMU2()
    mmu_filament_inserted,
#endif
#if HAS_TOOLCHANGER() || HAS_MMU2()
    tools_mapping,
#endif
    wrong_filament,
    file_error, ///< Something is wrong with the gcode file
    _last = file_error
};
constexpr inline ClientFSM client_fsm_from_phase(PhasesPrintPreview) { return ClientFSM::PrintPreview; }

#if HAS_SELFTEST()
// GUI phases of selftest/wizard
// WARNING: make sure that _first_xx and _last_xx are defined after normal selftest phases. This enum is exported by magic_enum library, and it has
// a limitation that only first item with same value is exported.
enum class PhasesSelftest : PhaseUnderlyingType {
    _none,

    Loadcell_prepare,
    Loadcell_move_away,
    Loadcell_tool_select,
    Loadcell_cooldown,
    Loadcell_user_tap_ask_abort,
    Loadcell_user_tap_countdown,
    Loadcell_user_tap_check,
    Loadcell_user_tap_ok,
    Loadcell_fail,
    _first_Loadcell = Loadcell_prepare,
    _last_Loadcell = Loadcell_fail,

    FSensor_ask_unload,
    FSensor_wait_tool_pick,
    FSensor_unload_confirm,
    FSensor_calibrate,
    FSensor_insertion_wait,
    FSensor_insertion_ok,
    FSensor_insertion_calibrate,
    Fsensor_enforce_remove,
    FSensor_done,
    FSensor_fail,
    _first_FSensor = FSensor_ask_unload,
    _last_FSensor = FSensor_fail,

    #if HAS_GEARS_CALIBRATION()
    GearsCalib_filament_check,
    GearsCalib_filament_loaded_ask_unload,
    GearsCalib_filament_unknown_ask_unload,
    GearsCalib_release_screws,
    GearsCalib_alignment,
    GearsCalib_tighten,
    GearsCalib_done,
    _first_GearsCalib = GearsCalib_filament_check,
    _last_GearsCalib = GearsCalib_done,
    #endif

    CalibZ,
    _first_CalibZ = CalibZ,
    _last_CalibZ = CalibZ,

    Axis,
    _first_Axis = Axis,
    _last_Axis = Axis,

    Heaters,
    HeatersDisabledDialog,
    Heaters_AskBedSheetAfterFail, ///< After bed heater selftest failed, this state prompts the user if he didn't forget to put on the print sheet
    _first_Heaters = Heaters,
    _last_Heaters = Heaters_AskBedSheetAfterFail,

    FirstLayer_mbl,
    FirstLayer_print,
    _first_FirstLayer = FirstLayer_mbl,
    _last_FirstLayer = FirstLayer_print,

    FirstLayer_filament_known_and_not_unsensed,
    FirstLayer_filament_not_known_or_unsensed,
    FirstLayer_calib,
    FirstLayer_use_val,
    FirstLayer_start_print,
    FirstLayer_reprint,
    FirstLayer_clean_sheet,
    FirstLayer_failed,
    _first_FirstLayerQuestions = FirstLayer_filament_known_and_not_unsensed,
    _last_FirstLayerQuestions = FirstLayer_failed,

    Dock_needs_calibration,
    Dock_move_away,
    Dock_wait_user_park1,
    Dock_wait_user_park2,
    Dock_wait_user_park3,
    Dock_wait_user_remove_pins,
    Dock_wait_user_loosen_pillar,
    Dock_wait_user_lock_tool,
    Dock_wait_user_tighten_top_screw,
    Dock_measure,
    Dock_wait_user_tighten_bottom_screw,
    Dock_wait_user_install_pins,
    Dock_selftest_park_test,
    Dock_selftest_failed,
    Dock_calibration_success,
    _first_Dock = Dock_needs_calibration,
    _last_Dock = Dock_calibration_success,

    ToolOffsets_wait_user_confirm_start,
    ToolOffsets_wait_user_clean_nozzle_cold,
    ToolOffsets_wait_user_clean_nozzle_hot,
    ToolOffsets_wait_user_install_sheet,
    ToolOffsets_pin_install_prepare,
    ToolOffsets_wait_user_install_pin,
    ToolOffsets_wait_stable_temp,
    ToolOffsets_wait_calibrate,
    ToolOffsets_wait_move_away,
    ToolOffsets_wait_user_remove_pin,
    _first_Tool_Offsets = ToolOffsets_wait_user_confirm_start,
    _last_Tool_Offsets = ToolOffsets_wait_user_remove_pin,

    RevisePrinterStatus_ask_revise, ///< Notifies that a selftest part failed and asks if the user wants to revise the setup
    RevisePrinterStatus_revise, ///< ScreenPrinterSetup being shown, user revising the printer setup
    RevisePrinterStatus_ask_retry, ///< After revision, ask the user to retry the selftest
    _first_RevisePrinterStatus = RevisePrinterStatus_ask_revise,
    _last_RevisePrinterStatus = RevisePrinterStatus_ask_retry,

    _last = _last_RevisePrinterStatus,
};
constexpr inline ClientFSM client_fsm_from_phase(PhasesSelftest) { return ClientFSM::Selftest; }
#endif

enum class PhaseNetworkSetup : PhaseUnderlyingType {
    init,

    ask_switch_to_wifi, ///< User is already connected through an ethernet cable, ask him if he wants to switch to wi-fi
    action_select, ///< Letting the user to choose how the wi-fi should be set up
    wifi_scan, ///< Scanning available wi-fi networks (the scanning is fully handled on the GUI thread)
    wait_for_ini_file, ///< Prompting user to insert a flash drive with creds
    ask_delete_ini_file, ///< Asking the user if he wants to delete the ini file
#if HAS_NFC()
    ask_use_prusa_app, ///< User is prompted if he wants to use the Prusa app to connect to the wi-fi
    wait_for_nfc, ///< Prompting user to provide the credentials through NFW
    nfc_confirm, ///< Loaded credentials via NFC, asking for confirmation
#endif
    connecting_finishable, ///< The user is connecting to a Wi-Fi. The screen offers a "Finish" button that keeps connecting on the background and "Cancel" to go back.
    connecting_nonfinishable, ///< The user is connecting to a Wi-Fi. The screen only offers a "Cancel" button to go back.
    connected,
    ask_setup_prusa_connect, ///< Prompts the user if he wants to set up Prusa Connect
    prusa_conect_setup, ///< Setup connect is running, waiting for it to finish

    no_interface_error,
    connection_error,
    help_qr, ///< Display as QR code to the help page

    finish,
    _last = finish,
};
constexpr inline ClientFSM client_fsm_from_phase(PhaseNetworkSetup) { return ClientFSM::NetworkSetup; }

#if ENABLED(CRASH_RECOVERY)
enum class PhasesCrashRecovery : PhaseUnderlyingType {
    check_X,
    check_Y,
    home,
    axis_NOK, //< just for unification of the two below
    axis_short,
    axis_long,
    repeated_crash,
    home_fail, //< Homing failed, ask to retry
    #if HAS_TOOLCHANGER()
    tool_recovery, //< Toolchanger recovery, tool fell off
    _last = tool_recovery
    #else
    _last = home_fail
    #endif
};
constexpr inline ClientFSM client_fsm_from_phase(PhasesCrashRecovery) { return ClientFSM::CrashRecovery; }
#endif

enum class PhasesQuickPause : PhaseUnderlyingType {
    QuickPaused,
    _last = QuickPaused
};
constexpr inline ClientFSM client_fsm_from_phase(PhasesQuickPause) { return ClientFSM::QuickPause; }

enum class PhasesWarning : PhaseUnderlyingType {
#if HAS_EMERGENCY_STOP()
    DoorOpen,
#endif
    // Generic warning with a Continue button, just for dismissing it.
    Warning,

// These have some actual buttons that need to be handled.
#if XL_ENCLOSURE_SUPPORT()
    EnclosureFilterExpiration,
#endif

    ProbingFailed,

    /// Shown when the M334 is attempting to change metrics configuration, prompting the user to confirm the change (security reasons)
    MetricsConfigChangePrompt,

    FilamentSensorStuckHelp,

#if HAS_MMU2()
    FilamentSensorStuckHelpMMU,
#endif

#if ENABLED(DETECT_PRINT_SHEET)
    /// Shown on failed print sheet detection. Custom handling.
    SteelSheetNotDetected,
#endif

#if HAS_CHAMBER_API()
    FailedToReachChamberTemperature,
#endif

#if HAS_UNEVEN_BED_PROMPT()
    /// A prompt offering Z align calibration when uneven bed is detected
    BedUnevenAlignmentPrompt,
#endif

    NozzleCleaningFailed,
    _last = NozzleCleaningFailed,
};
constexpr inline ClientFSM client_fsm_from_phase(PhasesWarning) { return ClientFSM::Warning; }

#if HAS_COLDPULL()
enum class PhasesColdPull : PhaseUnderlyingType {
    introduction,
    #if HAS_TOOLCHANGER()
    select_tool,
    pick_tool,
    #endif
    #if HAS_MMU2()
    stop_mmu,
    #endif
    #if HAS_TOOLCHANGER() || HAS_MMU2()
    unload_ptfe,
    load_ptfe,
    #endif
    prepare_filament,
    blank_load,
    blank_unload,
    cool_down,
    heat_up,
    automatic_pull,
    manual_pull,
    cleanup,
    pull_done,
    finish,
    _last = finish,
};
constexpr inline ClientFSM client_fsm_from_phase(PhasesColdPull) { return ClientFSM::ColdPull; }
#endif

#if HAS_PHASE_STEPPING()
enum class PhasesPhaseStepping : PhaseUnderlyingType {
    restore_defaults,
    intro,
    home,
    #if HAS_ATTACHABLE_ACCELEROMETER()
    connect_to_board,
    wait_for_extruder_temperature,
    attach_to_extruder,
    attach_to_bed,
    #endif
    calib_x,
    calib_y,
    calib_error,
    calib_nok,
    calib_ok,
    finish,
    _last = finish,
};
constexpr inline ClientFSM client_fsm_from_phase(PhasesPhaseStepping) { return ClientFSM::PhaseStepping; }
#endif

enum class PhasesFansSelftest : PhaseUnderlyingType {
    test_100_percent,
#if PRINTER_IS_PRUSA_MK3_5()
    manual_check,
#endif
    test_40_percent,
    results,
    _last = results,
};

constexpr inline ClientFSM client_fsm_from_phase(PhasesFansSelftest) { return ClientFSM::FansSelftest; }

#if HAS_INPUT_SHAPER_CALIBRATION()
enum class PhasesInputShaperCalibration : PhaseUnderlyingType {
    info,
    parking,
    #if HAS_ATTACHABLE_ACCELEROMETER()
    connect_to_board,
    wait_for_extruder_temperature,
    attach_to_extruder,
    attach_to_bed,
    #endif
    measuring_x_axis,
    measuring_y_axis,
    measurement_failed,
    computing,
    bad_results,
    results,
    finish,
    _last = finish,
};
constexpr inline ClientFSM client_fsm_from_phase(PhasesInputShaperCalibration) { return ClientFSM::InputShaperCalibration; }
#endif

enum class PhasesPrinting : PhaseUnderlyingType {
    active,
};
constexpr inline ClientFSM client_fsm_from_phase(PhasesPrinting) { return ClientFSM::Printing; }

enum class PhasesSerialPrinting : PhaseUnderlyingType {
    active,
};
constexpr inline ClientFSM client_fsm_from_phase(PhasesSerialPrinting) { return ClientFSM::Serial_printing; }

#if HAS_BELT_TUNING()
enum class PhaseBeltTuning : PhaseUnderlyingType {
    init,

    /// The user is prompted to loose the belts so that the gantry is aligned
    ask_for_gantry_align,

    /// Homing, selecting a proper tool and moving it to the measuring position
    preparing,

    /// Asking the user to install the dampeners
    ask_for_dampeners_installation,

    /// Calibrating accelerometer
    calibrating_accelerometer,

    /// Measuring the vibrations and such
    measuring,

    /// We vibrate on the highest measured peak frequency and let the user evaluate whether the belts are resonating.
    /// This is basically a measurement validity check
    vibration_check,

    /// Presenting the results to the user
    results,

    /// Asking the user to remove the dampeners
    ask_for_dampeners_uninstallation,

    /// Some error occured
    error,

    finish,
    _last = finish,
};
constexpr inline ClientFSM client_fsm_from_phase(PhaseBeltTuning) { return ClientFSM::BeltTuning; }
#endif

#if HAS_DOOR_SENSOR_CALIBRATION()
enum class PhaseDoorSensorCalibration : PhaseUnderlyingType {
    confirm_abort,
    repeat,
    skip_ask,
    confirm_closed,
    tighten_screw_half,
    confirm_open,
    loosen_screw_half,
    finger_test,
    tighten_screw_quarter,
    done,
    finish,
    _last = finish,
};
constexpr inline ClientFSM client_fsm_from_phase(PhaseDoorSensorCalibration) { return ClientFSM::DoorSensorCalibration; }
#endif

// static class for work with fsm responses (like button click)
// encode responses - get them from marlin client, to marlin server and decode them again
class ClientResponses {
    ClientResponses() = delete;
    ClientResponses(ClientResponses &) = delete;

    // declare 2d arrays of single buttons for radio buttons
    static constexpr EnumArray<PhasesLoadUnload, PhaseResponses, CountPhases<PhasesLoadUnload>()> LoadUnloadResponses {
        { PhasesLoadUnload::initial, {} },
            { PhasesLoadUnload::ChangingTool, {} },
            { PhasesLoadUnload::Parking_stoppable, { Response::Stop } },
            { PhasesLoadUnload::Parking_unstoppable, {} },
            { PhasesLoadUnload::WaitingTemp_stoppable, { Response::Stop } },
            { PhasesLoadUnload::WaitingTemp_unstoppable, {} },
            { PhasesLoadUnload::PreparingToRam_stoppable, { Response::Stop } },
            { PhasesLoadUnload::PreparingToRam_unstoppable, {} },
            { PhasesLoadUnload::Ramming_stoppable, { Response::Stop } },
            { PhasesLoadUnload::Ramming_unstoppable, {} },
            { PhasesLoadUnload::Unloading_stoppable, { Response::Stop } },
            { PhasesLoadUnload::Unloading_unstoppable, {} },
            { PhasesLoadUnload::RemoveFilament, { Response::Filament_removed } },
            { PhasesLoadUnload::IsFilamentUnloaded, { Response::Yes, Response::No } },
            { PhasesLoadUnload::FilamentNotInFS, { Response::Help } },
            { PhasesLoadUnload::ManualUnload_continuable, { Response::Continue, Response::Retry } },
            { PhasesLoadUnload::ManualUnload_uncontinuable, { Response::Help, Response::Retry } },
            { PhasesLoadUnload::UserPush_stoppable, { Response::Continue, Response::Stop } },
            { PhasesLoadUnload::UserPush_unstoppable, { Response::Continue } },
            { PhasesLoadUnload::MakeSureInserted_stoppable, { Response::Stop, Response::Help } },
            { PhasesLoadUnload::MakeSureInserted_unstoppable, { Response::Help } },
            { PhasesLoadUnload::Inserting_stoppable, { Response::Stop } },
            { PhasesLoadUnload::Inserting_unstoppable, {} },
            { PhasesLoadUnload::IsFilamentInGear, { Response::Yes, Response::No } },
            { PhasesLoadUnload::Ejecting_stoppable, { Response::Stop } },
            { PhasesLoadUnload::Ejecting_unstoppable, {} },
            { PhasesLoadUnload::Loading_stoppable, { Response::Stop } },
            { PhasesLoadUnload::Loading_unstoppable, {} },
            { PhasesLoadUnload::Purging_stoppable, { Response::Stop } },
            { PhasesLoadUnload::Purging_unstoppable, {} },
            { PhasesLoadUnload::IsColor, { Response::Yes, Response::Purge_more, Response::Retry } },
            { PhasesLoadUnload::IsColorPurge, { Response::Yes, Response::Purge_more } },
            { PhasesLoadUnload::Unparking, {} },
#if HAS_LOADCELL()
            { PhasesLoadUnload::FilamentStuck, { Response::Unload } },
#endif
#if HAS_AUTO_RETRACT()
            { PhasesLoadUnload::AutoRetracting, {} },
#endif
#if HAS_MMU2()
            { PhasesLoadUnload::LoadFilamentIntoMMU, { Response::Continue } },

            { PhasesLoadUnload::MMU_EngagingIdler, {} },
            { PhasesLoadUnload::MMU_DisengagingIdler, {} },
            { PhasesLoadUnload::MMU_UnloadingToFinda, {} },
            { PhasesLoadUnload::MMU_UnloadingToPulley, {} },
            { PhasesLoadUnload::MMU_FeedingToFinda, {} },
            { PhasesLoadUnload::MMU_FeedingToBondtech, {} },
            { PhasesLoadUnload::MMU_FeedingToNozzle, {} },
            { PhasesLoadUnload::MMU_AvoidingGrind, {} },
            { PhasesLoadUnload::MMU_FinishingMoves, {} },
            { PhasesLoadUnload::MMU_ERRDisengagingIdler, {} },
            { PhasesLoadUnload::MMU_ERREngagingIdler, {} },
            { PhasesLoadUnload::MMU_ERRWaitingForUser, { Response::Retry, Response::Slowly, Response::Continue, Response::Restart, Response::Unload, Response::Stop, Response::MMU_disable } },
            { PhasesLoadUnload::MMU_ERRInternal, {} },
            { PhasesLoadUnload::MMU_ERRHelpingFilament, {} },
            { PhasesLoadUnload::MMU_ERRTMCFailed, {} },
            { PhasesLoadUnload::MMU_UnloadingFilament, {} },
            { PhasesLoadUnload::MMU_LoadingFilament, {} },
            { PhasesLoadUnload::MMU_SelectingFilamentSlot, {} },
            { PhasesLoadUnload::MMU_PreparingBlade, {} },
            { PhasesLoadUnload::MMU_PushingFilament, {} },
            { PhasesLoadUnload::MMU_PerformingCut, {} },
            { PhasesLoadUnload::MMU_ReturningSelector, {} },
            { PhasesLoadUnload::MMU_ParkingSelector, {} },
            { PhasesLoadUnload::MMU_EjectingFilament, {} },
            { PhasesLoadUnload::MMU_RetractingFromFinda, {} },
            { PhasesLoadUnload::MMU_Homing, {} },
            { PhasesLoadUnload::MMU_MovingSelector, {} },
            { PhasesLoadUnload::MMU_FeedingToFSensor, {} },
            { PhasesLoadUnload::MMU_HWTestBegin, {} },
            { PhasesLoadUnload::MMU_HWTestIdler, {} },
            { PhasesLoadUnload::MMU_HWTestSelector, {} },
            { PhasesLoadUnload::MMU_HWTestPulley, {} },
            { PhasesLoadUnload::MMU_HWTestCleanup, {} },
            { PhasesLoadUnload::MMU_HWTestExec, {} },
            { PhasesLoadUnload::MMU_HWTestDisplay, {} },
            { PhasesLoadUnload::MMU_ErrHwTestFailed, {} },
#endif
    };

    static constexpr EnumArray<PhasesPreheat, PhaseResponses, CountPhases<PhasesPreheat>()> PreheatResponses {
        { PhasesPreheat::initial, {} },

        // Additionally, filament type selection is passed through FSMResponseVariant(FilamentType)
        { PhasesPreheat::UserTempSelection, { Response::Abort, Response::Cooldown } },
    };

    static constexpr EnumArray<PhasesPrintPreview, PhaseResponses, CountPhases<PhasesPrintPreview>()> PrintPreviewResponses {
        { PhasesPrintPreview::loading, {} },
            { PhasesPrintPreview::download_wait, {
                                                     Response::Quit,
                                                 } },
            { PhasesPrintPreview::main_dialog, {
#if PRINTER_IS_PRUSA_XL()
                                                   Response::Continue,
#elif PRINTER_IS_PRUSA_MINI()
                                                   Response::PRINT,
#else
                                                   Response::Print,
#endif
                                                   Response::Back,
                                               } },
            { PhasesPrintPreview::unfinished_selftest, {
                                                           Response::Ignore,
                                                           Response::Calibrate,
                                                       } },
            { PhasesPrintPreview::new_firmware_available, {
                                                              Response::Continue,
                                                          } },
            { PhasesPrintPreview::wrong_printer, {
                                                     Response::Abort,
                                                     Response::PRINT,
                                                 } },
            { PhasesPrintPreview::wrong_printer_abort, {
                                                           Response::Abort,
                                                       } },
            { PhasesPrintPreview::filament_not_inserted, {
                                                             Response::Yes,
                                                             Response::No,
                                                             Response::FS_disable,
                                                         } },
#if HAS_MMU2()
            { PhasesPrintPreview::mmu_filament_inserted, {
                                                             Response::Yes,
                                                             Response::No,
                                                         } },
#endif
#if HAS_TOOLCHANGER() || HAS_MMU2()
            { PhasesPrintPreview::tools_mapping, {
                                                     Response::Back,
                                                     Response::Filament,
                                                     Response::PRINT,
                                                 } },
#endif
            { PhasesPrintPreview::wrong_filament, {
#if !PRINTER_IS_PRUSA_XL()
                                                      Response::Change,
#endif
                                                      Response::Ok,
                                                      Response::Abort,
                                                  } },
            { PhasesPrintPreview::file_error, {
                                                  Response::Abort,
                                              } },
    };

    static constexpr EnumArray<PhasesSelftest, PhaseResponses, CountPhases<PhasesSelftest>()> SelftestResponses {
        { PhasesSelftest::_none, {} },
            { PhasesSelftest::Loadcell_prepare, {} },
            { PhasesSelftest::Loadcell_move_away, {} },
            { PhasesSelftest::Loadcell_tool_select, {} },
            { PhasesSelftest::Loadcell_cooldown, { Response::Abort } },

            { PhasesSelftest::Loadcell_user_tap_ask_abort, { Response::Continue, Response::Abort } },
            { PhasesSelftest::Loadcell_user_tap_countdown, {} },
            { PhasesSelftest::Loadcell_user_tap_check, {} },
            { PhasesSelftest::Loadcell_user_tap_ok, {} },
            { PhasesSelftest::Loadcell_fail, {} },

            { PhasesSelftest::FSensor_ask_unload, { Response::Continue, Response::Unload, Response::Abort } },
            { PhasesSelftest::FSensor_wait_tool_pick, {} },
            { PhasesSelftest::FSensor_unload_confirm, { Response::Yes, Response::No } },
            { PhasesSelftest::FSensor_calibrate, {} },
            { PhasesSelftest::FSensor_insertion_wait, { Response::Abort_invalidate_test } },
            { PhasesSelftest::FSensor_insertion_ok, { Response::Continue, Response::Abort_invalidate_test } },
            { PhasesSelftest::FSensor_insertion_calibrate, { Response::Abort_invalidate_test } },
            { PhasesSelftest::Fsensor_enforce_remove, { Response::Abort_invalidate_test } },
            { PhasesSelftest::FSensor_done, {} },
            { PhasesSelftest::FSensor_fail, {} },

#if HAS_GEARS_CALIBRATION()
            { PhasesSelftest::GearsCalib_filament_check, { Response::Continue, Response::Skip } },
            { PhasesSelftest::GearsCalib_filament_loaded_ask_unload, { Response::Unload, Response::Abort } },
            { PhasesSelftest::GearsCalib_filament_unknown_ask_unload, { Response::Continue, Response::Unload, Response::Abort } },
            { PhasesSelftest::GearsCalib_release_screws, { Response::Continue, Response::Skip } },
            { PhasesSelftest::GearsCalib_alignment, {} },
            { PhasesSelftest::GearsCalib_tighten, { Response::Continue } },
            { PhasesSelftest::GearsCalib_done, { Response::Continue } },
#endif

            { PhasesSelftest::CalibZ, {} },

            { PhasesSelftest::Axis, {} },

            { PhasesSelftest::Heaters, {} },
            { PhasesSelftest::HeatersDisabledDialog, { Response::Ok } },
            { PhasesSelftest::Heaters_AskBedSheetAfterFail, { Response::Ok, Response::Retry } },

            { PhasesSelftest::FirstLayer_mbl, {} },
            { PhasesSelftest::FirstLayer_print, {} },

            { PhasesSelftest::FirstLayer_filament_known_and_not_unsensed, { Response::Next, Response::Unload } },
            { PhasesSelftest::FirstLayer_filament_not_known_or_unsensed, { Response::Next, Response::Load, Response::Unload } },
            { PhasesSelftest::FirstLayer_calib, { Response::Next } },
            { PhasesSelftest::FirstLayer_use_val, { Response::Yes, Response::No } },
            { PhasesSelftest::FirstLayer_start_print, { Response::Next } },
            { PhasesSelftest::FirstLayer_reprint, { Response::Yes, Response::No } },
            { PhasesSelftest::FirstLayer_clean_sheet, { Response::Next } },
            { PhasesSelftest::FirstLayer_failed, { Response::Next } },

            { PhasesSelftest::Dock_needs_calibration, { Response::Continue, Response::Abort } },
            { PhasesSelftest::Dock_move_away, {} },
            { PhasesSelftest::Dock_wait_user_park1, { Response::Continue, Response::Abort } },
            { PhasesSelftest::Dock_wait_user_park2, { Response::Continue, Response::Abort } },
            { PhasesSelftest::Dock_wait_user_park3, { Response::Continue, Response::Abort } },
            { PhasesSelftest::Dock_wait_user_remove_pins, { Response::Continue, Response::Abort } },
            { PhasesSelftest::Dock_wait_user_loosen_pillar, { Response::Continue, Response::Abort } },
            { PhasesSelftest::Dock_wait_user_lock_tool, { Response::Continue, Response::Abort } },
            { PhasesSelftest::Dock_wait_user_tighten_top_screw, { Response::Continue, Response::Abort } },
            { PhasesSelftest::Dock_measure, { Response::Abort } },
            { PhasesSelftest::Dock_wait_user_tighten_bottom_screw, { Response::Continue, Response::Abort } },
            { PhasesSelftest::Dock_wait_user_install_pins, { Response::Continue, Response::Abort } },
            { PhasesSelftest::Dock_selftest_park_test, { Response::Abort } },
            { PhasesSelftest::Dock_selftest_failed, {} },
            { PhasesSelftest::Dock_calibration_success, { Response::Continue } },

            { PhasesSelftest::ToolOffsets_wait_user_confirm_start, { Response::Continue, Response::Abort } },
            { PhasesSelftest::ToolOffsets_wait_user_clean_nozzle_cold, { Response::Heatup, Response::Continue } },
            { PhasesSelftest::ToolOffsets_wait_user_clean_nozzle_hot, { Response::Cooldown, Response::Continue } },
            { PhasesSelftest::ToolOffsets_wait_user_install_sheet, { Response::Continue } },
            { PhasesSelftest::ToolOffsets_pin_install_prepare, {} },
            { PhasesSelftest::ToolOffsets_wait_user_install_pin, { Response::Continue } },
            { PhasesSelftest::ToolOffsets_wait_stable_temp, {} },
            { PhasesSelftest::ToolOffsets_wait_calibrate, {} },
            { PhasesSelftest::ToolOffsets_wait_move_away, {} },
            { PhasesSelftest::ToolOffsets_wait_user_remove_pin, { Response::Continue } },
            { PhasesSelftest::RevisePrinterStatus_ask_revise, { Response::Adjust, Response::Skip } },
            { PhasesSelftest::RevisePrinterStatus_revise, { Response::Done } },
            { PhasesSelftest::RevisePrinterStatus_ask_retry, { Response::Yes, Response::No } },
    };

    static constexpr EnumArray<PhaseNetworkSetup, PhaseResponses, CountPhases<PhaseNetworkSetup>()> network_setup_responses {
        { PhaseNetworkSetup::init, {} },
            { PhaseNetworkSetup::ask_switch_to_wifi, { Response::Yes, Response::No } },
            // Note: Additionally to this, the phase accepts various NetworkSetupResponse responses through FSMResponseVariant
            { PhaseNetworkSetup::action_select, { Response::Back, Response::Help } },
            // Note: Additionally to this, the phase accepts various NetworkSetupResponse responses through FSMResponseVariant
            { PhaseNetworkSetup::wifi_scan, { Response::Back } },
            { PhaseNetworkSetup::wait_for_ini_file, { Response::Cancel } },
            { PhaseNetworkSetup::ask_delete_ini_file, { Response::Yes, Response::No } },
#if HAS_NFC()
            { PhaseNetworkSetup::ask_use_prusa_app, { Response::Yes, Response::No } },
            { PhaseNetworkSetup::wait_for_nfc, { Response::Cancel } },
            { PhaseNetworkSetup::nfc_confirm, { Response::Ok, Response::Cancel } },
#endif
            { PhaseNetworkSetup::connecting_finishable, { Response::Finish, Response::Cancel } },
            { PhaseNetworkSetup::connecting_nonfinishable, { Response::Cancel } },
            { PhaseNetworkSetup::connected, { Response::Ok } },
            { PhaseNetworkSetup::ask_setup_prusa_connect, { Response::Yes, Response::No } },
            { PhaseNetworkSetup::prusa_conect_setup, { Response::Done } },

            { PhaseNetworkSetup::no_interface_error, { Response::Ok, Response::Help, Response::Retry } },
            { PhaseNetworkSetup::connection_error, { Response::Back, Response::Help, Response::Abort } },
            { PhaseNetworkSetup::help_qr, { Response::Back } },
            { PhaseNetworkSetup::finish, {} },
    };

#if ENABLED(CRASH_RECOVERY)
    static constexpr PhaseResponses CrashRecoveryResponses[] = {
        {}, // check X
        {}, // check Y
        {}, // home
        { Response::Retry, Response::Pause, Response::Resume }, // axis NOK
        {}, // axis short
        {}, // axis long
        { Response::Resume, Response::Pause }, // repeated crash
        { Response::Retry }, // home_fail
    #if HAS_TOOLCHANGER()
        { Response::Continue }, // toolchanger recovery
    #endif
    };
    static_assert(std::size(ClientResponses::CrashRecoveryResponses) == CountPhases<PhasesCrashRecovery>());
#endif

    static constexpr PhaseResponses QuickPauseResponses[] = {
        { Response::Resume }, // QuickPaused
    };
    static_assert(std::size(ClientResponses::QuickPauseResponses) == CountPhases<PhasesQuickPause>());

    static constexpr EnumArray<PhasesWarning, PhaseResponses, CountPhases<PhasesWarning>()> WarningResponses {
#if HAS_EMERGENCY_STOP()
        { PhasesWarning::DoorOpen, {} },
#endif
            { PhasesWarning::Warning, { Response::Continue } },
#if XL_ENCLOSURE_SUPPORT()
            { PhasesWarning::EnclosureFilterExpiration, { Response::Ignore, Response::Postpone5Days, Response::Done } },
#endif
            { PhasesWarning::ProbingFailed, { Response::Yes, Response::No } },
            { PhasesWarning::MetricsConfigChangePrompt, { Response::Yes, Response::No } },
            { PhasesWarning::FilamentSensorStuckHelp, { Response::Ok, Response::FS_disable } },
#if HAS_MMU2()
            { PhasesWarning::FilamentSensorStuckHelpMMU, { Response::Ok } },
#endif
#if ENABLED(DETECT_PRINT_SHEET)
            { PhasesWarning::SteelSheetNotDetected, { Response::Retry, Response::Ignore } },
#endif
#if HAS_CHAMBER_API()
            { PhasesWarning::FailedToReachChamberTemperature, { Response::Ok, Response::Skip } },
#endif
#if HAS_UNEVEN_BED_PROMPT()
            { PhasesWarning::BedUnevenAlignmentPrompt, { Response::Yes, Response::No } },
#endif
            { PhasesWarning::NozzleCleaningFailed, { Response::Retry, Response::Abort } },
    };

#if HAS_COLDPULL()
    static constexpr PhaseResponses ColdPullResponses[] = {
        { Response::Continue, Response::Stop }, // introduction,
    #if HAS_TOOLCHANGER()
        { Response::Continue, Response::Tool1, Response::Tool2, Response::Tool3, Response::Tool4, Response::Tool5 }, // select_tool
        {}, // pick_tool
    #endif
    #if HAS_MMU2()
        { Response::Abort }, // stop_mmu,
    #endif
    #if HAS_TOOLCHANGER() || HAS_MMU2()
        { Response::Unload, Response::Continue, Response::Abort }, // unload_ptfe,
        { Response::Load, Response::Continue, Response::Abort }, // load_ptfe,
    #endif
        { Response::Unload, Response::Load, Response::Continue, Response::Abort }, // prepare_filament,
        {}, // blank_load
        {}, // blank_unload
        { Response::Abort }, // cool_down,
        { Response::Abort }, // heat_up,
        {}, // automatic_pull,
        { Response::Continue }, // manual_pull,
        { Response::Abort }, // cleanup (restart_mmu),
        { Response::Finish }, // pull_done,
        {}, // finish,
    };
    static_assert(std::size(ClientResponses::ColdPullResponses) == CountPhases<PhasesColdPull>());
#endif

#if HAS_PHASE_STEPPING()
    static constexpr EnumArray<PhasesPhaseStepping, PhaseResponses, CountPhases<PhasesPhaseStepping>()> phase_stepping_calibration_responses {
        { PhasesPhaseStepping::restore_defaults, { Response::Ok } },
            { PhasesPhaseStepping::intro, { Response::Continue, Response::Abort } },
            { PhasesPhaseStepping::home, {} },
    #if HAS_ATTACHABLE_ACCELEROMETER()
            { PhasesPhaseStepping::connect_to_board, { Response::Abort } },
            { PhasesPhaseStepping::wait_for_extruder_temperature, { Response::Abort } },
            { PhasesPhaseStepping::attach_to_extruder, { Response::Continue, Response::Abort } },
            { PhasesPhaseStepping::attach_to_bed, { Response::Continue, Response::Abort } },
    #endif
            { PhasesPhaseStepping::calib_x, { Response::Abort } },
            { PhasesPhaseStepping::calib_y, { Response::Abort } },
            { PhasesPhaseStepping::calib_error, { Response::Ok } },
            { PhasesPhaseStepping::calib_nok, { Response::Ok } },
            { PhasesPhaseStepping::calib_ok, { Response::Ok } },
            { PhasesPhaseStepping::finish, {} },
    };
#endif

    static constexpr PhaseResponses FanSelftestResponses[] = {
        {}, // PhasesFanSelftest::test_100_percent
#if PRINTER_IS_PRUSA_MK3_5()
        { Response::Yes, Response::No }, // PhasesFanSelftest::manual_check
#endif
        {}, // PhasesFanSelftest::test_40_percent
        {}, // PhasesFanSelftest::results
    };
    static_assert(std::size(ClientResponses::FanSelftestResponses) == CountPhases<PhasesFansSelftest>());

#if HAS_INPUT_SHAPER_CALIBRATION()
    static constexpr EnumArray<PhasesInputShaperCalibration, PhaseResponses, CountPhases<PhasesInputShaperCalibration>()> input_shaper_calibration_responses {
        { PhasesInputShaperCalibration::info, { Response::Continue, Response::Abort } },
            { PhasesInputShaperCalibration::parking, {} },
    #if HAS_ATTACHABLE_ACCELEROMETER()
            { PhasesInputShaperCalibration::connect_to_board, { Response::Abort } },
            { PhasesInputShaperCalibration::wait_for_extruder_temperature, { Response::Abort } },
            { PhasesInputShaperCalibration::attach_to_extruder, { Response::Continue, Response::Abort } },
            { PhasesInputShaperCalibration::attach_to_bed, { Response::Continue, Response::Abort } },
    #endif
            { PhasesInputShaperCalibration::measuring_x_axis, { Response::Abort } },
            { PhasesInputShaperCalibration::measuring_y_axis, { Response::Abort } },
            { PhasesInputShaperCalibration::measurement_failed, { Response::Retry, Response::Abort } },
            { PhasesInputShaperCalibration::computing, { Response::Abort } },
            { PhasesInputShaperCalibration::bad_results, { Response::Ok } },
            { PhasesInputShaperCalibration::results, { Response::Yes, Response::No } },
            { PhasesInputShaperCalibration::finish, {} },
    };
#endif

#if HAS_BELT_TUNING()
    static constexpr EnumArray<PhaseBeltTuning, PhaseResponses, CountPhases<PhaseBeltTuning>()> belt_tuning_responses {
        { PhaseBeltTuning::init, {} },
        { PhaseBeltTuning::ask_for_gantry_align, { Response::Done, Response::Abort } },
        { PhaseBeltTuning::preparing, { Response::Abort } },
        { PhaseBeltTuning::ask_for_dampeners_installation, { Response::Done, Response::Abort } },
        { PhaseBeltTuning::calibrating_accelerometer, { Response::Abort } },
        { PhaseBeltTuning::measuring, { Response::Abort } },
        { PhaseBeltTuning::vibration_check, { Response::Yes, Response::No } },
        { PhaseBeltTuning::results, { Response::Retry, Response::Finish } },
        { PhaseBeltTuning::ask_for_dampeners_uninstallation, { Response::Done } },
        { PhaseBeltTuning::error, { Response::Abort, Response::Retry } },
        { PhaseBeltTuning::finish, {} },
    };
#endif

#if HAS_DOOR_SENSOR_CALIBRATION()
    static constexpr EnumArray<PhaseDoorSensorCalibration, PhaseResponses, CountPhases<PhaseDoorSensorCalibration>()> door_sensor_calibration_responses {
        { PhaseDoorSensorCalibration::confirm_abort, { Response::Back, Response::Skip } },
        { PhaseDoorSensorCalibration::repeat, { Response::Ok } },
        { PhaseDoorSensorCalibration::skip_ask, { Response::Calibrate, Response::Skip } },
        { PhaseDoorSensorCalibration::confirm_closed, { Response::Continue, Response::Abort } },
        { PhaseDoorSensorCalibration::tighten_screw_half, { Response::Continue, Response::Abort } },
        { PhaseDoorSensorCalibration::confirm_open, { Response::Continue, Response::Abort } },
        { PhaseDoorSensorCalibration::loosen_screw_half, { Response::Continue, Response::Abort } },
        { PhaseDoorSensorCalibration::finger_test, { Response::Continue, Response::Abort } },
        { PhaseDoorSensorCalibration::tighten_screw_quarter, { Response::Continue, Response::Abort } },
        { PhaseDoorSensorCalibration::done, { Response::Continue } },
        { PhaseDoorSensorCalibration::finish, {} },
    };
#endif

    static constexpr EnumArray<ClientFSM, std::span<const PhaseResponses>, ClientFSM::_count> fsm_phase_responses {
        { ClientFSM::Serial_printing, {} },
            { ClientFSM::Load_unload, LoadUnloadResponses },
            { ClientFSM::Preheat, PreheatResponses },
#if HAS_SELFTEST()
            { ClientFSM::Selftest, SelftestResponses },
#endif
            { ClientFSM::FansSelftest, FanSelftestResponses },
            { ClientFSM::NetworkSetup, network_setup_responses },
            { ClientFSM::Printing, {} },
#if ENABLED(CRASH_RECOVERY)
            { ClientFSM::CrashRecovery, CrashRecoveryResponses },
#endif
            { ClientFSM::QuickPause, QuickPauseResponses },
            { ClientFSM::Warning, WarningResponses },
            { ClientFSM::PrintPreview, PrintPreviewResponses },
#if HAS_COLDPULL()
            { ClientFSM::ColdPull, ColdPullResponses },
#endif
#if HAS_PHASE_STEPPING()
            { ClientFSM::PhaseStepping, phase_stepping_calibration_responses },
#endif
#if HAS_INPUT_SHAPER_CALIBRATION()
            { ClientFSM::InputShaperCalibration, input_shaper_calibration_responses },
#endif
#if HAS_BELT_TUNING()
            { ClientFSM::BeltTuning, belt_tuning_responses },
#endif
#if HAS_DOOR_SENSOR_CALIBRATION()
            { ClientFSM::DoorSensorCalibration, door_sensor_calibration_responses },
#endif
            { ClientFSM::Wait, {} },
    };

public:
    static constexpr const PhaseResponses &get_fsm_responses(ClientFSM fsm_type, PhaseUnderlyingType phase) {
        if (ftrstd::to_underlying(fsm_type) >= fsm_phase_responses.size()) {
            return empty_phase_responses;
        }

        const auto &responses = fsm_phase_responses[fsm_type];
        if (phase >= responses.size()) {
            return empty_phase_responses;
        }

        return responses[phase];
    }

    // get all responses accepted in phase
    static constexpr const PhaseResponses &GetResponses(FSMAndPhase fsm_phase) {
        return get_fsm_responses(fsm_phase.fsm, fsm_phase.phase);
    }

    // get index of single response in PhaseResponses
    // return -1 (maxval) if does not exist
    static constexpr uint8_t GetIndex(FSMAndPhase fsm_phase, Response response) {
        const auto responses = fsm_phase_responses[fsm_phase.fsm];
        if (fsm_phase.phase >= responses.size()) {
            return -1;
        }

        const PhaseResponses &cmds = responses[fsm_phase.phase];
        for (size_t i = 0; i < MAX_RESPONSES; ++i) {
            if (cmds[i] == response) {
                return i;
            }
        }
        return -1;
    }

    // get response from PhaseResponses by index
    static constexpr const Response &GetResponse(FSMAndPhase phase, const uint8_t index) {
        if (index >= MAX_RESPONSES) {
            return ResponseNone;
        }
        return GetResponses(phase)[index];
    }

    template <class T>
    static bool HasButton(const T phase) {
        return GetResponse(phase, 0) != Response::_none; // this phase has no responses
    }
};

enum class SelftestParts {
    Axis,
#if HAS_LOADCELL()
    Loadcell,
#endif
    CalibZ,
    Heaters,
#if FILAMENT_SENSOR_IS_ADC()
    FSensor,
#endif
#if HAS_GEARS_CALIBRATION()
    GearsCalib,
#endif
    FirstLayer,
    FirstLayerQuestions,
#if HAS_TOOLCHANGER()
    Dock,
    ToolOffsets,
#endif
    RevisePrinterSetup,
    _none, // cannot be created, must have same index as _count
    _count = _none
};

static constexpr PhasesSelftest SelftestGetFirstPhaseFromPart(SelftestParts part) {
    switch (part) {
    case SelftestParts::Axis:
        return PhasesSelftest::_first_Axis;
#if HAS_LOADCELL()
    case SelftestParts::Loadcell:
        return PhasesSelftest::_first_Loadcell;
#endif
#if FILAMENT_SENSOR_IS_ADC()
    case SelftestParts::FSensor:
        return PhasesSelftest::_first_FSensor;
#endif
#if HAS_GEARS_CALIBRATION()
    case SelftestParts::GearsCalib:
        return PhasesSelftest::_first_GearsCalib;
#endif
    case SelftestParts::CalibZ:
        return PhasesSelftest::_first_CalibZ;
    case SelftestParts::Heaters:
        return PhasesSelftest::_first_Heaters;
    case SelftestParts::FirstLayer:
        return PhasesSelftest::_first_FirstLayer;
    case SelftestParts::FirstLayerQuestions:
        return PhasesSelftest::_first_FirstLayerQuestions;
#if HAS_TOOLCHANGER()
    case SelftestParts::Dock:
        return PhasesSelftest::_first_Dock;
    case SelftestParts::ToolOffsets:
        return PhasesSelftest::_first_Tool_Offsets;
#endif
    case SelftestParts::RevisePrinterSetup:
        return PhasesSelftest::_first_RevisePrinterStatus;

    case SelftestParts::_none:
        break;
    }
    return PhasesSelftest::_none;
}

static constexpr PhasesSelftest SelftestGetLastPhaseFromPart(SelftestParts part) {
    switch (part) {
    case SelftestParts::Axis:
        return PhasesSelftest::_last_Axis;
#if HAS_LOADCELL()
    case SelftestParts::Loadcell:
        return PhasesSelftest::_last_Loadcell;
#endif
#if FILAMENT_SENSOR_IS_ADC()
    case SelftestParts::FSensor:
        return PhasesSelftest::_last_FSensor;
#endif
#if HAS_GEARS_CALIBRATION()
    case SelftestParts::GearsCalib:
        return PhasesSelftest::_last_GearsCalib;
#endif
    case SelftestParts::CalibZ:
        return PhasesSelftest::_last_CalibZ;
    case SelftestParts::Heaters:
        return PhasesSelftest::_last_Heaters;
    case SelftestParts::FirstLayer:
        return PhasesSelftest::_last_FirstLayer;
    case SelftestParts::FirstLayerQuestions:
        return PhasesSelftest::_last_FirstLayerQuestions;
#if HAS_TOOLCHANGER()
    case SelftestParts::Dock:
        return PhasesSelftest::_last_Dock;
    case SelftestParts::ToolOffsets:
        return PhasesSelftest::_last_Tool_Offsets;
#endif
    case SelftestParts::RevisePrinterSetup:
        return PhasesSelftest::_last_RevisePrinterStatus;

    case SelftestParts::_none:
        break;
    }
    return PhasesSelftest::_none;
}

static constexpr bool SelftestPartContainsPhase(SelftestParts part, PhasesSelftest ph) {
    const PhaseUnderlyingType ph_u16 = PhaseUnderlyingType(ph);

    return (ph_u16 >= PhaseUnderlyingType(SelftestGetFirstPhaseFromPart(part))) && (ph_u16 <= PhaseUnderlyingType(SelftestGetLastPhaseFromPart(part)));
}

static constexpr SelftestParts SelftestGetPartFromPhase(PhasesSelftest ph) {
    for (size_t i = 0; i < size_t(SelftestParts::_none); ++i) {
        if (SelftestPartContainsPhase(SelftestParts(i), ph)) {
            return SelftestParts(i);
        }
    }

#if HAS_LOADCELL()
    if (SelftestPartContainsPhase(SelftestParts::Loadcell, ph)) {
        return SelftestParts::Loadcell;
    }
#endif

#if FILAMENT_SENSOR_IS_ADC()
    if (SelftestPartContainsPhase(SelftestParts::FSensor, ph)) {
        return SelftestParts::FSensor;
    }
#endif
#if HAS_GEARS_CALIBRATION()
    if (SelftestPartContainsPhase(SelftestParts::GearsCalib, ph)) {
        return SelftestParts::GearsCalib;
    }
#endif
    if (SelftestPartContainsPhase(SelftestParts::Axis, ph)) {
        return SelftestParts::Axis;
    }

    if (SelftestPartContainsPhase(SelftestParts::Heaters, ph)) {
        return SelftestParts::Heaters;
    }

    if (SelftestPartContainsPhase(SelftestParts::CalibZ, ph)) {
        return SelftestParts::CalibZ;
    }

#if BOARD_IS_XLBUDDY()
    if (SelftestPartContainsPhase(SelftestParts::Dock, ph)) {
        return SelftestParts::Dock;
    }
#endif

    if (SelftestPartContainsPhase(SelftestParts::RevisePrinterSetup, ph)) {
        return SelftestParts::RevisePrinterSetup;
    }

    return SelftestParts::_none;
};
