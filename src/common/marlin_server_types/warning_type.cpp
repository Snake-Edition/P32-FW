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

#if XL_ENCLOSURE_SUPPORT()
    case WarningType::EnclosureFilterExpiration:
        return PhasesWarning::EnclosureFilterExpiration;
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

        //
    }
}

PhasesWarning warning_type_phase(WarningType warning) {
    return warning_type_phase_constexpr(warning);
}

static_assert([] {
    std::bitset<CountPhases<PhasesWarning>()> used_phases;

    // Check that each phase (except for Warning, which is handled separately) has a separate phase
    // If this does not apply and we use
    // In the future, we could possibly unify WarningType and PhasesWarning
    for (size_t i = 0; i <= static_cast<size_t>(WarningType::_last); i++) {
        const WarningType wt = static_cast<WarningType>(i);
        const PhasesWarning ph = warning_type_phase_constexpr(wt);
        const auto phi = std::to_underlying(ph);

        if (ph != PhasesWarning::Warning && used_phases.test(phi)) {
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
