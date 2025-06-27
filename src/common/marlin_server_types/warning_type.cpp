#include "warning_type.hpp"

#include <bitset>

constexpr PhasesWarning warning_type_phase_constexpr(WarningType warning) {
    switch (warning) {

    default:
        // Intentionally returning Warning by default - only a few warnings use different phase
        return PhasesWarning::Warning;

    case WarningType::MetricsConfigChangePrompt:
        return PhasesWarning::MetricsConfigChangePrompt;

    case WarningType::ProbingFailed:
        return PhasesWarning::ProbingFailed;

    case WarningType::FilamentSensorStuckHelp:
        return PhasesWarning::FilamentSensorStuckHelp;

#if HAS_MMU2()
    case WarningType::FilamentSensorStuckHelpMMU:
        return PhasesWarning::FilamentSensorStuckHelpMMU;
#endif

#if HAS_LOADCELL() && ENABLED(PROBE_CLEANUP_SUPPORT)
    case WarningType::NozzleCleaningFailed:
        return PhasesWarning::NozzleCleaningFailed;
#endif

#if HAS_UNEVEN_BED_PROMPT()
    case WarningType::BedUnevenAlignmentPrompt:
        return PhasesWarning::BedUnevenAlignmentPrompt;
#endif

#if XL_ENCLOSURE_SUPPORT() || HAS_CHAMBER_FILTRATION_API()
    case WarningType::EnclosureFilterExpiration:
        return PhasesWarning::EnclosureFilterExpiration;
#endif

#if HAS_MANUAL_CHAMBER_VENTS()
    case WarningType::OpenChamberVents:
        return PhasesWarning::ChamberVents;
    case WarningType::CloseChamberVents:
        return PhasesWarning::ChamberVents;
#endif

#if HAS_EMERGENCY_STOP()
    case WarningType::DoorOpen:
        return PhasesWarning::DoorOpen;
#endif

#if HAS_CHAMBER_API()
    case WarningType::FailedToReachChamberTemperature:
        return PhasesWarning::FailedToReachChamberTemperature;
#endif

#if ENABLED(DETECT_PRINT_SHEET)
    case WarningType::SteelSheetNotDetected:
        return PhasesWarning::SteelSheetNotDetected;
#endif

#if HAS_CEILING_CLEARANCE()
    case WarningType::CeilingClearanceViolation:
        return PhasesWarning::CeilingClearanceViolation;
#endif

#if HAS_PRECISE_HOMING_COREXY()
    case WarningType::HomingCalibrationNeeded:
        return PhasesWarning::HomingCalibrationNeeded;

    case WarningType::HomingRefinementFailed:
        return PhasesWarning::HomingRefinementFailed;

    case WarningType::HomingRefinementFailedNoRetry:
        return PhasesWarning::HomingRefinementFailedNoRetry;
#endif

        //
    }
}

PhasesWarning warning_type_phase(WarningType warning) {
    return warning_type_phase_constexpr(warning);
}

constexpr uint32_t warning_lifespan_sec_constexpr(WarningType type) {
    switch (type) {
#if HAS_MANUAL_CHAMBER_VENTS()
    case WarningType::OpenChamberVents:
    case WarningType::CloseChamberVents:
        return 60;
#endif
    default:
        return uint32_t(-1); // Unlimited
    }
}

uint32_t warning_lifespan_sec(WarningType type) {
    return warning_lifespan_sec_constexpr(type);
}

static_assert([] {
    std::bitset<CountPhases<PhasesWarning>()> used_phases;

    // Check that each phase (except for Warning and ChamberVents, which are handled separately) has a separate phase
    // If this does not apply and we use
    // In the future, we could possibly unify WarningType and PhasesWarning
    for (size_t i = 0; i < std::to_underlying(WarningType::_cnt); i++) {
        const WarningType wt = static_cast<WarningType>(i);
        const PhasesWarning ph = warning_type_phase_constexpr(wt);
        const auto phi = std::to_underlying(ph);

        bool phase_warning_exception = ph == PhasesWarning::Warning;
#if HAS_MANUAL_CHAMBER_VENTS()
        phase_warning_exception |= ph == PhasesWarning::ChamberVents;
#endif

        if (!phase_warning_exception && used_phases.test(phi)) {
            std::abort();
        }

        used_phases.set(phi);
    }

    // Check that every phase is used by some warning - otherwise it's pointless
    for (size_t i = 0; i < CountPhases<PhasesWarning>(); i++) {
        if (!used_phases.test(i)) {
            std::abort();
        }
    }

    return true;
}());
