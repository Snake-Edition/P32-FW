#pragma once

#include "client_response.hpp"
#include <option/has_manual_chamber_vents.h>
#include <option/has_remote_bed.h>
#include <option/has_chamber_filtration_api.h>
#include <option/xbuddy_extension_variant_standard.h>
#include <option/has_selftest.h>
#include <option/has_precise_homing_corexy.h>

enum class WarningType : uint32_t {
#if HAS_EMERGENCY_STOP()
    DoorOpen,
#endif
    HotendFanError,
    PrintFanError,
    HeatersTimeout,
    HotendTempDiscrepancy,
    NozzleTimeout,
    FilamentLoadingTimeout,
    FilamentSensorStuckHelp,
#if HAS_MMU2()
    FilamentSensorStuckHelpMMU,
    MaintenanceWarningFails,
    MaintenanceWarningChanges,
#endif
    FilamentSensorsDisabled,
#if _DEBUG
    SteppersTimeout,
#endif
    USBFlashDiskError,
    USBDriveUnsupportedFileSystem,
#if HAS_SELFTEST()
    ActionSelftestRequired,
#endif
#if ENABLED(POWER_PANIC)
    HeatbedColdAfterPP,
#endif
    HeatBreakThermistorFail,
#if ENABLED(CALIBRATION_GCODE)
    NozzleDoesNotHaveRoundSection,
#endif
    BuddyMCUMaxTemp,
#if HAS_DWARF()
    DwarfMCUMaxTemp,
#endif
#if HAS_REMOTE_BED()
    BedMCUMaxTemp,
#endif
    ProbingFailed,
#if HAS_LOADCELL() && ENABLED(PROBE_CLEANUP_SUPPORT)
    NozzleCleaningFailed,
#endif
#if XL_ENCLOSURE_SUPPORT() || HAS_CHAMBER_FILTRATION_API()
    EnclosureFilterExpirWarning,
    EnclosureFilterExpiration,
#endif
#if XL_ENCLOSURE_SUPPORT()
    EnclosureFanError,
#endif
#if ENABLED(DETECT_PRINT_SHEET)
    SteelSheetNotDetected,
#endif
    NotDownloaded,
    GcodeCorruption,
    GcodeCropped,
    MetricsConfigChangePrompt,
#if HAS_CHAMBER_API()
    FailedToReachChamberTemperature,
#endif
#if HAS_MANUAL_CHAMBER_VENTS()
    OpenChamberVents,
    CloseChamberVents,
#endif
#if HAS_UNEVEN_BED_PROMPT()
    BedUnevenAlignmentPrompt,
#endif
#if HAS_CHAMBER_API()
    ChamberOverheatingTemperature,
    ChamberCriticalTemperature,
#endif
#if XBUDDY_EXTENSION_VARIANT_STANDARD()
    ChamberCoolingFanError,
#endif
#if XBUDDY_EXTENSION_VARIANT_STANDARD() || XL_ENCLOSURE_SUPPORT()
    ChamberFiltrationFanError,
#endif
#if HAS_CEILING_CLEARANCE()
    CeilingClearanceViolation,
#endif
#if HAS_PRECISE_HOMING_COREXY()
    HomingCalibrationNeeded,
    HomingRefinementFailed,
    HomingRefinementFailedNoRetry,
#endif
    AccelerometerCommunicationFailed,
    _last = AccelerometerCommunicationFailed,
};

PhasesWarning warning_type_phase(WarningType warning);

uint32_t warning_lifespan_sec(WarningType type);
