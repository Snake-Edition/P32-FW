#include "manual_belt_tuning_wizard.hpp"
#include "manual_belt_tuning_config.hpp"

#include <Marlin/src/Marlin.h>
#include <Marlin/src/gcode/calibrate/M958.hpp>
#include <feature/motordriver_util.h>
#include <feature/phase_stepping/phase_stepping.hpp>

#include <option/has_xbuddy_extension.h>
#if HAS_XBUDDY_EXTENSION()
    #include <feature/xbuddy_extension/xbuddy_extension.hpp>
#endif

#include <fsm/manual_belt_tuning_phases.hpp>
#include <common/marlin_server.hpp>
#include <bsod/bsod.h>
#include <string_builder.hpp>

#include <Marlin/src/gcode/gcode.h>
#include <Marlin/src/module/motion.h>

#include <cmath>

#if !PRINTER_IS_PRUSA_COREONE()
    #error "A lot of magic number values in this file are CoreONE specific. Belt tuning has not been tested on any other printers, hence the values are probably not correct for them. Revise them at manual_belt_tuning_config.hpp"
#endif

using namespace marlin_server;
using namespace manual_belt_tuning;

namespace {

class ManualBeltTuningWizard {
private:
    enum class Result {
        abort,
        retry,
        pass,
    };

    phase_stepping::EnsureDisabled phase_stepping_disabler;

public:
    ManualBeltTuningWizard() {}
    ~ManualBeltTuningWizard() {
#if HAS_XBUDDY_EXTENSION()
        buddy::xbuddy_extension().set_strobe(std::nullopt);
#else
    #error "This is a vital part of the calibration, without LED stroboscop this calibration is not reasonable (user won't see the correct resonating frequency)"
#endif
        disable_all_steppers();
    }

    Result run_from_gantry() {
        MicrostepRestorer microstep_restorer;
        disable_all_steppers();
        fsm_change(PhaseManualBeltTuning::check_x_gantry);
        if (wait_for_continue(PhaseManualBeltTuning::check_x_gantry)) {
            fsm_change(PhaseManualBeltTuning::homing_wait);
        } else {
            return Result::abort;
        }

        enable_all_steppers();
        if (!GcodeSuite::G28_no_parser(true, true, true, { .precise = false })) {
            return Result::abort;
        }
        do_blocking_move_to(240, -8, 10, 50.f); // belt tension calibration position
        planner.synchronize();

        fsm_change(PhaseManualBeltTuning::intro_measure);
        if (wait_for_continue(PhaseManualBeltTuning::intro_measure)) {
            const struct belt_tensions tension_data(freq_top_belt_optimal, freq_bottom_belt_optimal);
            fsm_change(PhaseManualBeltTuning::measure_upper_belt, fsm::serialize_data<belt_tensions>(tension_data));
        } else {
            return Result::abort;
        }

        // Taken from M958::setup_axis
        // enable all axes to have the same state as printing
        enable_all_steppers();
        stepper_microsteps(X_AXIS, 128);
        stepper_microsteps(Y_AXIS, 128);

        Vibrate vibrator {
            .frequency = 80,
            .excitation_acceleration = 2.5f,
            .axis_flag = STEP_EVENT_FLAG_STEP_X | STEP_EVENT_FLAG_STEP_Y | STEP_EVENT_FLAG_Y_DIR, // Vibrate the toolhead front and back
        };

        if (!vibrator.setup(microstep_restorer)) {
            return Result::abort;
        }

        // Variables for result frequency
        float freq_top_belt = freq_top_belt_optimal;
        float freq_bottom_belt = freq_bottom_belt_optimal;

        // Measurement checkpoint, make sure fsm with phase data is set up
        while (true) {
            struct belt_tensions tension_data(freq_top_belt, freq_bottom_belt);
            fsm_change(PhaseManualBeltTuning::measure_upper_belt, fsm::serialize_data<belt_tensions>(tension_data));
            get_last_knob_move(); // Consume last recorded move

            vibrator.frequency = freq_top_belt;
            vibrator.excitation_acceleration = abs(calc_accel(vibrator.frequency) * 0.001f);

            // Contains loop that breaks with knob click
            resonate(vibrator, PhaseManualBeltTuning::measure_upper_belt);

            // Save top belt results
            freq_top_belt = vibrator.frequency;
            tension_data = belt_tensions(freq_top_belt, freq_bottom_belt);
            fsm_change(PhaseManualBeltTuning::measure_lower_belt, fsm::serialize_data<belt_tensions>(tension_data));

            vibrator.frequency = freq_bottom_belt;
            vibrator.excitation_acceleration = abs(calc_accel(vibrator.frequency) * 0.001f);

            // Contains loop that breaks with knob click
            resonate(vibrator, PhaseManualBeltTuning::measure_lower_belt);

            // Save bottom belt results
            freq_bottom_belt = vibrator.frequency;
            tension_data = belt_tensions(freq_top_belt, freq_bottom_belt);

            if (freq_top_belt < freq_result_min || freq_top_belt > freq_result_max
                || freq_bottom_belt < freq_result_min || freq_bottom_belt > freq_result_max) {
                fsm_change(PhaseManualBeltTuning::alignment_issue);
                switch (wait_for_response(PhaseManualBeltTuning::alignment_issue)) {
                case Response::Retry:
                    return Result::retry;
                case Response::Abort:
                    return Result::abort;
                default:
                    bsod_unreachable();
                    return Result::abort;
                }
            }

            // difference frequency
            const float dft = freq_top_belt_optimal - freq_top_belt;
            const float dfb = freq_bottom_belt_optimal - freq_bottom_belt;
            // difference revelations
            const float drt = calc_revs_from_freq(dft, dfb, belt_hz_per_rev, belt_hz_per_rev2);
            const float drb = calc_revs_from_freq(dfb, dft, belt_hz_per_rev, belt_hz_per_rev2);
            //  average
            const float dr = (drt + drb) / 2;
            // eights of one turn (signed)
            const int dri8 = int(dr * 8);

            fsm_change(PhaseManualBeltTuning::show_tension, fsm::serialize_data<belt_tensions>(tension_data));
            if (wait_for_continue(PhaseManualBeltTuning::show_tension)) {
                struct screw_revs revs_data = {
                    .turn_eights = static_cast<int8_t>(dri8),
                };
                fsm_change(PhaseManualBeltTuning::adjust_tensioners, fsm::serialize_data<screw_revs>(revs_data));
            } else {
                return Result::abort;
            }

            // Adjust tensioners
            switch (wait_for_response(PhaseManualBeltTuning::adjust_tensioners)) {
            case Response::Continue:
                fsm_change(PhaseManualBeltTuning::finished);
                break;
            case Response::Adjust:
                continue; // Restart measurements from top belt
            case Response::Abort:
                return Result::abort;
            default:
                bsod_unreachable();
                break;
            }
            break;
        }
        return Result::pass;
    }

    void run() {

        planner.finish_and_disable();
        FSM_Holder holder { PhaseManualBeltTuning::intro };
        if (!wait_for_continue(PhaseManualBeltTuning::intro)) {
            return;
        }

        Result res = Result::abort;
        do {
            // Starting top belt tension
            res = run_from_gantry();
        } while (res == Result::retry);

        if (res == Result::abort) {
            return;
        }

        // Single button finish screen
        wait_for_response(PhaseManualBeltTuning::finished);
        config_store().manual_belt_tuning_completed.set(true); // TODO: Unused, for the moment manual belt tuning is hardcoded in settings
    }

    void adjust_vibrator(Vibrate &vibrator) {
        vibrator.frequency = std::clamp<float>(abs(vibrator.frequency), freq_min, freq_max);
        vibrator.excitation_acceleration = abs(calc_accel(vibrator.frequency) * 0.001f);
#if HAS_XBUDDY_EXTENSION()
        buddy::xbuddy_extension().set_strobe(vibrator.frequency + 4 /*taken from python script*/);
#endif
    }

    Response resonate(Vibrate &vibrator, PhaseManualBeltTuning phase) {
        Response response = Response::_none;
        while (true) {
            const auto knob_move = get_last_knob_move();
            if (knob_move != KnobMove::NoMove) {
                vibrator.frequency += knob_move == KnobMove::Up ? 0.5f : -0.5f;
            }
            adjust_vibrator(vibrator);

            const struct belt_tensions tension_data(vibrator.frequency, vibrator.frequency); // duplicated - could be either of the belts
            fsm_change(phase, fsm::serialize_data<belt_tensions>(tension_data));

            vibrator.step();

            if ((response = get_response_from_phase(phase)) != Response::_none) {
                break;
            }

            idle(true);
        }
        return response;
    }

    bool wait_for_continue(PhaseManualBeltTuning phase) const {
        switch (wait_for_response(phase)) {
        case Response::Continue:
            return true;
        case Response::Abort:
            break;
        default:
            bsod_unreachable();
            break;
        }
        return false;
    }
};

} // namespace

void manual_belt_tuning::run_wizard() {
    ManualBeltTuningWizard wizard;
    wizard.run();
}
