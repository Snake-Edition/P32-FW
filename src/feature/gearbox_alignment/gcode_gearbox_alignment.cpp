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
#include <Marlin/src/gcode/gcode.h>

namespace {
class GearboxAlignmentWizard {
public:
    static constexpr PhaseGearboxAlignment first_phase = PhaseGearboxAlignment::intro;

    GearboxAlignmentWizard(const uint8_t _tool = 0)
        : tool(_tool)
        , holder(first_phase) {
    }

    void execute() {
        do {
            do_phase_and_set_next();
        } while (curr_phase != PhaseGearboxAlignment::finish);
    }

private:
    const uint8_t tool;
    marlin_server::FSM_Holder holder;
    PhaseGearboxAlignment curr_phase = first_phase;

    PhaseGearboxAlignment get_next_phase(const PhaseGearboxAlignment phase) {
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

    PhaseGearboxAlignment fsm_change(PhaseGearboxAlignment phase) {
        curr_phase = phase;
        marlin_server::fsm_change(phase);
        return phase;
    }

    PhaseGearboxAlignment finish(TestResult test_result) {
        SelftestTool selftest_tool = config_store().get_selftest_result_tool(tool);
        selftest_tool.gears = test_result;
        config_store().set_selftest_result_tool(tool, selftest_tool);
        return fsm_change(PhaseGearboxAlignment::finish);
    }

    void move_gear() {
        AutoRestore<bool> CE(thermalManager.allow_cold_extrude);
        thermalManager.allow_cold_extrude = true;
        constexpr const float feedrate = 8;
        mapi::extruder_schedule_turning(feedrate, -0.6);
    }

    PhaseGearboxAlignment filament_check() {
        if (IFSensor *sensor = GetExtruderFSensor(tool)) {
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

    PhaseGearboxAlignment filament_unload() {
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

    PhaseGearboxAlignment intro() {
        switch (marlin_server::wait_for_response(PhaseGearboxAlignment::intro)) {
        case Response::Skip:
            // Skipped gearvox alignment is considered passed;
            // this is meant for users with prebuilt printer.
            return finish(TestResult_Passed);
        case Response::Continue:
            tool_change(tool);
            return filament_check();
        default:
            break;
        }
        BUDDY_UNREACHABLE();
    }

    PhaseGearboxAlignment filament_loaded_ask_unload() {
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

    PhaseGearboxAlignment filament_unknown_ask_unload() {
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

    PhaseGearboxAlignment loosen_screws() {
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

    PhaseGearboxAlignment alignment() {
        uint32_t ticks = ticks_ms();
        while (ticks_ms() - ticks < 20'000) {
            move_gear();
            idle(true);
        }
        return fsm_change(PhaseGearboxAlignment::tighten_screws);
    }

    PhaseGearboxAlignment tighten_screws() {
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

    PhaseGearboxAlignment done() {
        switch (marlin_server::wait_for_response(PhaseGearboxAlignment::done)) {
        case Response::Continue:
            return finish(TestResult_Passed);
        default:
            break;
        }
        BUDDY_UNREACHABLE();
    }
};
}; // namespace
/** \addtogroup G-Codes
 * @{
 */

/**
 *### M1979 [ T ]: Gearbox alignment dialog (internal GCode)
 *
 *  - `T` - Set the tool
 *           1. if tool is not specified, active toolhead is picked
 *           2. if tool is out of range, gcode will do nothing
 *
 *#### Usage
 *
 *    M1979 T2 ; Run the gearbox alignment wizard on toolhead 2
 *    M1979    ; Run gearbox alignment wizard on active toolhead
 *
 */
void PrusaGcodeSuite::M1979() {
    int8_t target_tool = GcodeSuite::get_target_extruder_from_command();

    // Wrong value specified in T argument
    if (target_tool == -1) {
        return;
    }
    GearboxAlignmentWizard ga(target_tool);
    ga.execute();
}

/** @}*/
