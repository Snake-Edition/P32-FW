#include "config_features.h"
#include "filament_sensors_handler.hpp"
#include <config_store/store_instance.hpp>

// clang-format off
#if (!ENABLED(FILAMENT_LOAD_UNLOAD_GCODES)) || \
    HAS_LCD_MENU || \
    ENABLED(MIXING_EXTRUDER) || \
    ENABLED(NO_MOTION_BEFORE_HOMING)
    #error unsupported
#endif
// clang-format on

#include "../../../lib/Marlin/Marlin/src/Marlin.h"
#include "../../../lib/Marlin/Marlin/src/module/motion.h"
#include "../../../lib/Marlin/Marlin/src/module/planner.h"
#include "../../../lib/Marlin/Marlin/src/module/temperature.h"
#include "pause_stubbed.hpp"
#include "filament_sensors_handler.hpp"
#include "M70X.hpp"

#if HAS_CHAMBER_API()
    #include <feature/chamber/chamber.hpp>
#endif

static FSMResponseVariant preheatTempKnown(uint8_t target_extruder) {
    const FilamentType filament_type = config_store().get_filament_type(target_extruder);
    assert(filament_type != FilamentType::none);
    return FSMResponseVariant::make(filament_type);
}

static FSMResponseVariant preheatTempUnKnown(PreheatData preheat_data, bool break_on_autoload = false) {
    marlin_server::FSM_Holder holder { PhasesPreheat::UserTempSelection, preheat_data.serialize() };

    while (true) {
        if (const auto ret = marlin_server::get_response_variant_from_phase(PhasesPreheat::UserTempSelection)) {
            return ret;
        }
        if (preheat_data.mode == PreheatMode::Autoload && FSensors_instance().sensor_state(LogicalFilamentSensor::autoload) == FilamentSensorState::NoFilament) {
            return FSMResponseVariant::make(Response::Abort);
        }
        if (break_on_autoload && FSensors_instance().IsAutoloadInProgress()) {
            return FSMResponseVariant();
        }

        idle(true, true);
    }
}

static FSMResponseVariant evaluate_preheat_conditions(PreheatData preheat_data, uint8_t target_extruder) {
    bool canKnowTemp = preheat_data.mode == PreheatMode::Unload || preheat_data.mode == PreheatMode::Change_phase1 || preheat_data.mode == PreheatMode::Purge || preheat_data.mode == PreheatMode::Unload_askUnloaded;

    // Check if we are using operation which can get temp from printer and check if it can get the temp from available info (inserted filament or set temperature in temperature menu and no filament inserted)
    if (canKnowTemp && ((config_store().get_filament_type(target_extruder) != FilamentType::none))) {
        // We can get temperature without user telling us
        return preheatTempKnown(target_extruder);

    } else {
        // we need to ask the user for temperature
        return preheatTempUnKnown(preheat_data);
    }
}

std::pair<std::optional<PreheatStatus::Result>, FilamentType> filament_gcodes::preheat(PreheatData preheat_data, uint8_t target_extruder, PreheatBehavior preheat_arg) {
    const FSMResponseVariant response = evaluate_preheat_conditions(preheat_data, target_extruder);

    if (response.holds_alternative<FilamentType>()) {
        const FilamentType filament = response.value<FilamentType>();
        preheat_to(filament, target_extruder, preheat_arg);
        return { std::nullopt, filament };
    }

    switch (response.value_or<Response>(Response::_none)) {

    case Response::Abort:
        return { PreheatStatus::Result::Aborted, FilamentType::none };

    case Response::Cooldown:
        return { PreheatStatus::Result::CooledDown, FilamentType::none };

    default:
        // should not happen
        return { PreheatStatus::Result::Error, FilamentType::none };
    }
}

void filament_gcodes::preheat_to(FilamentType filament, uint8_t target_extruder, PreheatBehavior preheat_arg) {
    const FilamentTypeParameters fil_cnf = filament.parameters();

    // change temp only if it is lower than currently loaded filament
    if (preheat_arg.force_temp || thermalManager.degTargetHotend(target_extruder) < fil_cnf.nozzle_temperature) {
        thermalManager.setTargetHotend(fil_cnf.nozzle_temperature, target_extruder);

        const uint8_t extruder =
#if HAS_MMU2()
            // MMU has multiple slots (target_extruder can be >0) but only a single nozzle
            // -> we need the correct temperature per active slot
            // but we also need marlin_server::set_temp_to_display not to crash due to target_extruder > 0
            0;

#else
            target_extruder;
#endif

        marlin_server::set_temp_to_display(fil_cnf.nozzle_temperature, extruder);
        if (preheat_arg.preheat_bed && (preheat_arg.force_temp || (thermalManager.degTargetBed() < fil_cnf.heatbed_temperature))) {
            thermalManager.setTargetBed(fil_cnf.heatbed_temperature);
        }
    }

#if HAS_CHAMBER_API()
    if (preheat_arg.set_chamber_temperature) {
        buddy::chamber().set_target_temperature(fil_cnf.chamber_target_temperature);
    }
#endif
}

std::pair<std::optional<PreheatStatus::Result>, FilamentType> filament_gcodes::preheat_for_change_load(PreheatData data, uint8_t target_extruder) {
    const FSMResponseVariant response = preheatTempUnKnown(data);

    if (response.holds_alternative<FilamentType>()) {
        const FilamentType filament = response.value<FilamentType>();
        // change temp every time (unlike normal preheat)
        preheat_to(filament, target_extruder, PreheatBehavior::force_preheat_bed_and_chamber(config_store().filament_change_preheat_all.get()));
        return { std::nullopt, filament };
    }

    switch (response.value_or<Response>(Response::_none)) {

    case Response::Abort:
        return { PreheatStatus::Result::Aborted, FilamentType::none };

    case Response::Cooldown:
        return { PreheatStatus::Result::CooledDown, FilamentType::none };

    default:
        // should not happen
        return { PreheatStatus::Result::Error, FilamentType::none };
    }
}

void filament_gcodes::M1700_no_parser(const M1700Args &args) {
    InProgress progress;
    const FSMResponseVariant response_variant = preheatTempUnKnown(PreheatData::make(args.mode, args.target_extruder, args.preheat), true);

    // autoload ocurred
    if (!response_variant) {
        return;
    }

    const Response response = response_variant.value_or<Response>(Response::_none);
    if (response == Response::Abort) {
        PreheatStatus::SetResult(PreheatStatus::Result::Aborted);
        return;
    }

    const FilamentType filament = response_variant.value_or<FilamentType>(FilamentType::none);
    const FilamentTypeParameters fil_cnf = filament.parameters();

    const auto set_extruder_temp = [&](uint8_t extruder) {
        thermalManager.setTargetHotend(args.enforce_target_temp ? fil_cnf.nozzle_temperature : fil_cnf.nozzle_preheat_temperature, extruder);
        marlin_server::set_temp_to_display(fil_cnf.nozzle_temperature, extruder);
    };

    if (response == Response::Cooldown || args.target_extruder < 0) {
        // Set temperature to all tools
        // Cooldown is always applied to all tools
        HOTEND_LOOP() {
#if ENABLED(PRUSA_TOOLCHANGER)
            if (!prusa_toolchanger.is_tool_enabled(e)) {
                continue;
            }
#endif /*ENABLED(PRUSA_TOOLCHANGER)*/
            set_extruder_temp(e);
        }

    } else {
        // Preheat only target tool
        set_extruder_temp(args.target_extruder);
    }

    if (args.preheat_bed) {
        thermalManager.setTargetBed(fil_cnf.heatbed_temperature);
    }

#if HAS_CHAMBER_API()
    if (args.preheat_chamber) {
        buddy::chamber().set_target_temperature(fil_cnf.chamber_target_temperature);
    }
#endif

    // cooldown pressed
    if (filament == FilamentType::none) {
        thermalManager.set_fan_speed(0, 0);

    } else if ((axis_homed & _BV(Z_AXIS)) != _BV(Z_AXIS)) {
        unhomed_z_lift(10);
    }

    if (args.save) {
        if (args.target_extruder < 0) {
            HOTEND_LOOP() {
                config_store().set_filament_type(e, filament);
            }
        } else {
            config_store().set_filament_type(args.target_extruder, filament);
        }
    }

    // store result, so other threads can see it
    PreheatStatus::SetResult(PreheatStatus::Result::DoneNoFilament);

    // we might want to set filament type even with preheat, if so do:
    // Filaments::SetToBeLoaded(filament);
}
