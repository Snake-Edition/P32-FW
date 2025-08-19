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
#include <buddy/unreachable.hpp>
#include <string_builder.hpp>

#include <Marlin/src/gcode/gcode.h>
#include <Marlin/src/module/motion.h>

#include <cmath>

#if !PRINTER_IS_PRUSA_COREONE()
    #error "A lot of magic number values in this file are CoreONE specific. Belt tuning has not been tested on any other printers, hence the values are probably not correct for them. Proceed at your own risk."
#endif

using namespace marlin_server;
using namespace manual_belt_tuning;

namespace {

class ManualBeltTuningWizard {
public:
    ManualBeltTuningWizard() {}
    ~ManualBeltTuningWizard() {
#if HAS_XBUDDY_EXTENSION()
        buddy::xbuddy_extension().set_strobe(std::nullopt);
#endif
    }

    void run() {
        // phstep needs to be off _before_ getting the current ustep resolution
        phase_stepping::EnsureDisabled phaseSteppingDisabler;
        MicrostepRestorer microstepRestorer;

        planner.finish_and_disable();

        FSM_Holder holder { PhaseManualBeltTuning::intro };

        if (wait_for_continue(PhaseManualBeltTuning::intro)) {
            fsm_change(PhaseManualBeltTuning::check_x_gantry);
        } else {
            return;
        }

        if (wait_for_continue(PhaseManualBeltTuning::check_x_gantry)) {
            fsm_change(PhaseManualBeltTuning::homing_wait);
        } else {
            return;
        }

        if (!GcodeSuite::G28_no_parser(true, true, true, { .precise = true })) {
            return;
        }
        do_blocking_move_to(240, -8, 10, 50.f); // belt tension calibration position
        planner.synchronize();

        fsm_change(PhaseManualBeltTuning::intro_measure);
        if (wait_for_continue(PhaseManualBeltTuning::intro_measure)) {
            struct belt_tensions tension_data = {
                .upper_belt_tension = static_cast<uint8_t>(freq_top_belt_optimal)
            };
            fsm_change(PhaseManualBeltTuning::measure_upper_belt, fsm::serialize_data<belt_tensions>(tension_data));
        } else {
            return;
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

        if (!vibrator.setup(microstepRestorer)) {
            return;
        }

        bool skip_upper_belt = false;

        // Variables for result frequency
        float freq_top_belt = freq_top_belt_optimal;
        float freq_bottom_belt = freq_bottom_belt_optimal;

        while (true) {
            get_last_knob_move(); // Consume last recorded move

            vibrator.frequency = freq_top_belt;
            if (!skip_upper_belt) {
                vibrator.excitation_acceleration = abs(calc_accel(vibrator.frequency) * 0.001f);

                // Contains loop that breaks with knob click
                resonate(vibrator, PhaseManualBeltTuning::measure_upper_belt);

                // Save top belt results
                freq_top_belt = vibrator.frequency;
                struct belt_tensions tension_data = {
                    .lower_belt_tension = static_cast<uint8_t>(freq_bottom_belt)
                };
                fsm_change(PhaseManualBeltTuning::measure_lower_belt, fsm::serialize_data<belt_tensions>(tension_data));
            }

            vibrator.frequency = freq_bottom_belt;

            vibrator.excitation_acceleration = abs(calc_accel(vibrator.frequency) * 0.001f);
            resonate(vibrator, PhaseManualBeltTuning::measure_lower_belt);

            // Save bottom belt results
            freq_bottom_belt = vibrator.frequency;

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

            struct belt_tensions tension_data = {
                .upper_belt_tension = static_cast<uint8_t>(freq_top_belt),
                .lower_belt_tension = static_cast<uint8_t>(freq_bottom_belt)
            };

            fsm_change(PhaseManualBeltTuning::show_tension, fsm::serialize_data<belt_tensions>(tension_data));

            struct screw_revs revs_data = {
                .turn_eights = static_cast<int8_t>(dri8),
            };

            if (wait_for_continue(PhaseManualBeltTuning::show_tension)) {
                fsm_change(PhaseManualBeltTuning::adjust_tensioners, fsm::serialize_data<screw_revs>(revs_data));
            } else {
                return;
            }

            if (wait_for_continue(PhaseManualBeltTuning::adjust_tensioners)) {
                fsm_change(PhaseManualBeltTuning::intro_recheck_target_freq);
            } else {
                return;
            }

            if (wait_for_continue(PhaseManualBeltTuning::intro_recheck_target_freq)) {
                fsm_change(PhaseManualBeltTuning::recheck_upper_belt);
            } else {
                return;
            }

            auto response = resonate(vibrator, PhaseManualBeltTuning::recheck_upper_belt);
            if (response == Response::Adjust) {
                fsm_change(PhaseManualBeltTuning::measure_upper_belt);
                skip_upper_belt = false;
                continue;
            } else {
                fsm_change(PhaseManualBeltTuning::recheck_lower_belt);
            }

            response = resonate(vibrator, PhaseManualBeltTuning::recheck_lower_belt);
            if (response == Response::Adjust) {
                fsm_change(PhaseManualBeltTuning::measure_lower_belt);
                skip_upper_belt = true;
                continue;
            } else {
                config_store().manual_belt_tuning_completed.set(true); // TODO: Ask Martin from where wizard should be run and use this persistent variable
                break;
            }
        }
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
            switch (phase) {
            case PhaseManualBeltTuning::measure_upper_belt:
            case PhaseManualBeltTuning::measure_lower_belt: {
                const auto knob_move = get_last_knob_move();
                if (knob_move != KnobMove::NoMove) {
                    vibrator.frequency += knob_move == KnobMove::Up ? 0.5f : -0.5f;

                    struct belt_tensions tension_data = {
                        .upper_belt_tension = static_cast<uint8_t>(vibrator.frequency),
                        .lower_belt_tension = static_cast<uint8_t>(vibrator.frequency), // duplicated - could be either of the belts
                    };
                    fsm_change(phase, fsm::serialize_data<belt_tensions>(tension_data));
                }
                break;
            }
            case PhaseManualBeltTuning::recheck_upper_belt:
                vibrator.frequency = freq_top_belt_optimal;
                break;
            case PhaseManualBeltTuning::recheck_lower_belt:
                vibrator.frequency = freq_bottom_belt_optimal;
                break;
            default:
                BUDDY_UNREACHABLE();
                break;
            }
            adjust_vibrator(vibrator);

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
            BUDDY_UNREACHABLE();
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
