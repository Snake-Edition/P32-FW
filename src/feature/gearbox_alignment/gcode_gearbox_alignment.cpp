/// @file
#include "gcode_gearbox_alignment.hpp"

#include <buddy/unreachable.hpp>
#include <common/filament_sensors_handler.hpp>
#include <common/RAII.hpp>
#include <config_store/store_instance.hpp>
#include <M70X.hpp>
#include <mapi/motion.hpp>
#include <Marlin/src/Marlin.h>
#include <Marlin/src/module/temperature.h>

static PhaseGearboxAlignment fsm_change(PhaseGearboxAlignment phase) {
    marlin_server::fsm_change(phase);
    return phase;
}

static PhaseGearboxAlignment finish(TestResult test_result) {
    SelftestResult selftest_result = config_store().selftest_result.get();
    selftest_result.gears = test_result;
    config_store().selftest_result.set(selftest_result);
    return fsm_change(PhaseGearboxAlignment::finish);
}

static void move_gear() {
    AutoRestore<bool> CE(thermalManager.allow_cold_extrude);
    thermalManager.allow_cold_extrude = true;
    constexpr const float feedrate = 8;
    mapi::extruder_schedule_turning(feedrate, -0.6);
}

static PhaseGearboxAlignment filament_check() {
    if (IFSensor *sensor = GetExtruderFSensor(0)) {
        switch (sensor->get_state()) {
        case FilamentSensorState::HasFilament:
            return fsm_change(PhaseGearboxAlignment::filament_loaded_ask_unload);
        case FilamentSensorState::NoFilament:
            return fsm_change(PhaseGearboxAlignment::loosen_screws);
        case FilamentSensorState::NotInitialized:
        case FilamentSensorState::NotCalibrated:
        case FilamentSensorState::NotConnected:
        case FilamentSensorState::Disabled:
            break;
        }
    }
    return fsm_change(PhaseGearboxAlignment::filament_unknown_ask_unload);
}

static PhaseGearboxAlignment filament_unload() {
    const uint8_t target_extruder = active_extruder;

    filament_gcodes::M702_no_parser(std::nullopt, Z_AXIS_LOAD_POS, RetAndCool_t::Return, target_extruder, false);

    // check if we returned from preheat or finished the unload
    if (PreheatStatus::ConsumeResult() == PreheatStatus::Result::DoneNoFilament) {
        Temperature::setTargetHotend(0, target_extruder);
        Temperature::setTargetBed(0);
        return fsm_change(PhaseGearboxAlignment::loosen_screws);
    } else {
        return filament_check();
    }
}

static PhaseGearboxAlignment intro() {
    switch (marlin_server::wait_for_response(PhaseGearboxAlignment::intro)) {
    case Response::Skip:
        // Skipped gearvox alignment is considered passed;
        // this is meant for users with prebuilt printer.
        return finish(TestResult_Passed);
    case Response::Continue:
        return filament_check();
    default:
        break;
    }
    BUDDY_UNREACHABLE();
}

static PhaseGearboxAlignment filament_loaded_ask_unload() {
    switch (marlin_server::wait_for_response(PhaseGearboxAlignment::filament_loaded_ask_unload)) {
    case Response::Abort:
        return finish(TestResult_Skipped);
    case Response::Unload:
        return filament_unload();
    default:
        break;
    }
    BUDDY_UNREACHABLE();
}

static PhaseGearboxAlignment filament_unknown_ask_unload() {
    switch (marlin_server::wait_for_response(PhaseGearboxAlignment::filament_unknown_ask_unload)) {
    case Response::Abort:
        return finish(TestResult_Skipped);
    case Response::Unload:
        return filament_unload();
    case Response::Continue:
        return fsm_change(PhaseGearboxAlignment::loosen_screws);
    default:
        break;
    }
    BUDDY_UNREACHABLE();
}

static PhaseGearboxAlignment loosen_screws() {
    switch (marlin_server::wait_for_response(PhaseGearboxAlignment::loosen_screws)) {
    case Response::Continue:
        return fsm_change(PhaseGearboxAlignment::alignment);
    case Response::Skip:
        return finish(TestResult_Skipped);
    default:
        break;
    }
    BUDDY_UNREACHABLE();
}

static PhaseGearboxAlignment alignment() {
    uint32_t ticks = ticks_ms();
    while (ticks_ms() - ticks < 20'000) {
        move_gear();
        idle(true);
    }
    return fsm_change(PhaseGearboxAlignment::tighten_screws);
}

static PhaseGearboxAlignment tighten_screws() {
    for (;;) {
        switch (marlin_server::get_response_from_phase(PhaseGearboxAlignment::tighten_screws)) {
        case Response::_none:
            move_gear();
            idle(true);
            continue;
        case Response::Continue:
            return fsm_change(PhaseGearboxAlignment::done);
        default:
            continue;
        }
    }
}

static PhaseGearboxAlignment done() {
    switch (marlin_server::wait_for_response(PhaseGearboxAlignment::done)) {
    case Response::Continue:
        return finish(TestResult_Passed);
    default:
        break;
    }
    BUDDY_UNREACHABLE();
}

static PhaseGearboxAlignment get_next_phase(const PhaseGearboxAlignment phase) {
    switch (phase) {
    case PhaseGearboxAlignment::intro:
        return intro();
    case PhaseGearboxAlignment::filament_loaded_ask_unload:
        return filament_loaded_ask_unload();
    case PhaseGearboxAlignment::filament_unknown_ask_unload:
        return filament_unknown_ask_unload();
    case PhaseGearboxAlignment::loosen_screws:
        return loosen_screws();
    case PhaseGearboxAlignment::alignment:
        return alignment();
    case PhaseGearboxAlignment::tighten_screws:
        return tighten_screws();
    case PhaseGearboxAlignment::done:
        return done();
    case PhaseGearboxAlignment::finish:
        break;
    }
    BUDDY_UNREACHABLE();
}

/** \addtogroup G-Codes
 * @{
 */

/**
 *### M1979: Gearbox alignment dialog
 *
 * Internal GCode
 *
 *#### Usage
 *
 *    M1979
 *
 */
void PrusaGcodeSuite::M1979() {
    PhaseGearboxAlignment phase = PhaseGearboxAlignment::intro;
    marlin_server::FSM_Holder holder { phase };
    do {
        phase = get_next_phase(phase);
    } while (phase != PhaseGearboxAlignment::finish);
}

/** @}*/
