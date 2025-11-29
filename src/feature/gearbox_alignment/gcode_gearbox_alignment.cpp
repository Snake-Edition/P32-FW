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

    void do_phase_and_set_next() {
        switch (curr_phase) {
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

    void fsm_change(PhaseGearboxAlignment phase) {
        curr_phase = phase;
        marlin_server::fsm_change(phase);
    }

    void finish(TestResult test_result) {
        SelftestTool selftest_tool = config_store().get_selftest_result_tool(tool);
        selftest_tool.gears = test_result;
        config_store().set_selftest_result_tool(tool, selftest_tool);
        fsm_change(PhaseGearboxAlignment::finish);
        return;
    }

    void move_gear() {
        AutoRestore<bool> CE(thermalManager.allow_cold_extrude);
        thermalManager.allow_cold_extrude = true;
        constexpr const float feedrate = 8;
        mapi::extruder_schedule_turning(feedrate, -0.6);
    }

    void filament_check() {
        if (IFSensor *sensor = GetExtruderFSensor(tool)) {
            switch (sensor->get_state()) {
            case FilamentSensorState::HasFilament:
                fsm_change(PhaseGearboxAlignment::filament_loaded_ask_unload);
                return;
            case FilamentSensorState::NoFilament:
                fsm_change(PhaseGearboxAlignment::loosen_screws);
                return;
            case FilamentSensorState::NotInitialized:
            case FilamentSensorState::NotCalibrated:
            case FilamentSensorState::NotConnected:
            case FilamentSensorState::Disabled:
                break;
            }
        }
        fsm_change(PhaseGearboxAlignment::filament_unknown_ask_unload);
        return;
    }

    void filament_unload() {
        const uint8_t target_extruder = active_extruder;

        filament_gcodes::M702_no_parser(std::nullopt, Z_AXIS_LOAD_POS, RetAndCool_t::Return, target_extruder, false);

        // check if we returned from preheat or finished the unload
        if (PreheatStatus::ConsumeResult() == PreheatStatus::Result::DoneNoFilament) {
            Temperature::setTargetHotend(0, target_extruder);
            Temperature::setTargetBed(0);
            fsm_change(PhaseGearboxAlignment::loosen_screws);
        } else {
            filament_check();
        }
    }

    void intro() {
        switch (marlin_server::wait_for_response(PhaseGearboxAlignment::intro)) {
        case Response::Skip:
            // Skipped gearvox alignment is considered passed;
            // this is meant for users with prebuilt printer.
            finish(TestResult_Passed);
            return;
        case Response::Continue:
#if HAS_TOOLCHANGER()
            tool_change(tool);
#endif
            filament_check();
            return;
        default:
            break;
        }
        BUDDY_UNREACHABLE();
    }

    void filament_loaded_ask_unload() {
        switch (marlin_server::wait_for_response(PhaseGearboxAlignment::filament_loaded_ask_unload)) {
        case Response::Abort:
            finish(TestResult_Skipped);
            return;
        case Response::Unload:
            filament_unload();
            return;
        default:
            break;
        }
        BUDDY_UNREACHABLE();
    }

    void filament_unknown_ask_unload() {
        switch (marlin_server::wait_for_response(PhaseGearboxAlignment::filament_unknown_ask_unload)) {
        case Response::Abort:
            finish(TestResult_Skipped);
            return;
        case Response::Unload:
            filament_unload();
            return;
        case Response::Continue:
            fsm_change(PhaseGearboxAlignment::loosen_screws);
            return;
        default:
            break;
        }
        BUDDY_UNREACHABLE();
    }

    void loosen_screws() {
        switch (marlin_server::wait_for_response(PhaseGearboxAlignment::loosen_screws)) {
        case Response::Continue:
            fsm_change(PhaseGearboxAlignment::alignment);
            return;
        case Response::Skip:
            finish(TestResult_Skipped);
            return;
        default:
            break;
        }
        BUDDY_UNREACHABLE();
    }

    void alignment() {
        uint32_t ticks = ticks_ms();
        while (ticks_ms() - ticks < 20'000) {
            move_gear();
            idle(true);
        }
        fsm_change(PhaseGearboxAlignment::tighten_screws);
        return;
    }

    void tighten_screws() {
        for (;;) {
            switch (marlin_server::get_response_from_phase(PhaseGearboxAlignment::tighten_screws)) {
            case Response::_none:
                move_gear();
                idle(true);
                continue;
            case Response::Continue:
                fsm_change(PhaseGearboxAlignment::done);
                return;
            default:
                continue;
            }
        }
    }

    void done() {
        switch (marlin_server::wait_for_response(PhaseGearboxAlignment::done)) {
        case Response::Continue:
            finish(TestResult_Passed);
            return;
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
