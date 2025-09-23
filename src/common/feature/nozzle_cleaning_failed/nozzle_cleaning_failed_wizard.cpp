#include "nozzle_cleaning_failed_wizard.hpp"
#include <buddy/unreachable.hpp>
#include <feature/auto_retract/auto_retract.hpp>
#include <gcode/temperature/M104_M109.hpp>
#include <mapi/motion.hpp>
#include <fsm/nozzle_cleaning_failed_phases.hpp>

#include <Marlin/src/Marlin.h>
#include <common/marlin_server.hpp>
#include <gcode/gcode.h>

using namespace marlin_server;
using namespace nozzle_cleaning_failed_wizard;
using Phase = PhaseNozzleCleaningFailed;

namespace {

class NozzleCleaningFailedWizard {
private:
    FSM_Holder fsm;

    /**
     * @brief To avoid drawing over the warning if its active
     */
    bool warn_active;

public:
    NozzleCleaningFailedWizard()
        : fsm(Phase::init)
        , warn_active(false) {}

    Result run() {
        // Inform the user that the nozzle cleaning failed, ask him what to do (ignore/retry/abort)
        switch (ask_user_what_to_do()) {

        case Response::Retry:
            // Continue the wizard
            break;

        case Response::Ignore:
            // Ignore the failure, do not continue the wizard
            return Result::ignore;

        case Response::Abort:
            print_abort();
            return Result::abort;

        default:
            BUDDY_UNREACHABLE();
        }

        // In case we have nozzle cleaner OR we are missing autoretract, no sense in normal purging so return
#if HAS_NOZZLE_CLEANING_FAILED_PURGING()
        // Recommend to do the purge sequence
        if (recommend_purge()) {
            const float temp_before = Temperature::degTargetHotend(active_extruder);
            const float target_temp = config_store().get_filament_type(active_extruder).parameters().nozzle_temperature;

            // Heat up the nozzle
            if (!wait_temp(target_temp)) {
                print_abort();
                return Result::abort;
            }

            // Purge
            if (!purge()) {
                print_abort();
                return Result::abort;
            }

            // Autoretract - no way to abort that one
            autoretract();

            // Ask the user to clean the nozzle from the ooze
            if (!ask_user_to_clean_nozzle()) {
                print_abort();
                return Result::abort;
            }

            // Cool down to the original temperature
            if (!wait_temp(temp_before)) {
                print_abort();
                return Result::abort;
            }
        }
#endif

        return Result::retry; // The wizard is done -> retry nozzle cleaning
    }

    [[nodiscard]] Response ask_user_what_to_do() {
        while (true) {
            fsm_change(Phase::cleaning_failed);
            const auto response = marlin_server::wait_for_response(Phase::cleaning_failed);
            if (response == Response::Abort && !confirm_abort(Phase::cleaning_failed)) {
                // User selected Abort but changed his mind -> ask the question again
                continue;
            }
            return response;
        }
    }

#if HAS_NOZZLE_CLEANING_FAILED_PURGING()
    [[nodiscard]] bool recommend_purge() {
        fsm_change(Phase::recommend_purge);
        const auto response = marlin_server::wait_for_response(Phase::recommend_purge);
        switch (response) {
        case Response::Yes:
            return true;
        case Response::No:
            return false;
        default:
            BUDDY_UNREACHABLE();
        }
    }

    [[nodiscard]] bool wait_temp(float target_temp) {
        fsm_change(Phase::wait_temp);
        float start_temp = Temperature::degHotend(active_extruder);

        struct {
            float start_temp;
            float target_temp;
        } temps {
            start_temp,
            target_temp
        };

        // takes care of progress reporing and also handles abort correctly
        CallbackHookGuard subscriber(marlin_server::idle_hook_point, [&temps, this] {
            if (marlin_server::get_response_from_phase(Phase::wait_temp) == Response::Abort) {
                if (confirm_abort(Phase::wait_temp)) {
                    // Interrupt the M109
                    planner.quick_stop();
                }
            }
            if (!warn_active) {
                struct NozzleCleaningFailedProgressData data {
                    .progress_0_255 = static_cast<uint8_t>(std::min(255.f, fabs(static_cast<float>(Temperature::degHotend(active_extruder) - temps.start_temp) / static_cast<float>(temps.target_temp - temps.start_temp)) * 255))
                };
                fsm_change(Phase::wait_temp, fsm::serialize_data(data));
            }
        });

        // Wait for temp
        M109Flags flags {
            .target_temp = target_temp,
            .wait_heat_or_cool = true,
            .autotemp = true,
        };
        M109_no_parser(active_extruder, flags);
        return !planner.draining();
    }

    [[nodiscard]] bool purge() {
        fsm_change(Phase::purge);
        constexpr int16_t purge_length = 20;

        const int16_t total_length = buddy::auto_retract().retracted_distance().value_or(0) + purge_length; // We need to ensure we purge even if the value is invalid (nullopt)
        int16_t reference_e_pos = marlin_vars().native_pos[MARLIN_VAR_INDEX_E];

        CallbackHookGuard subscriber(marlin_server::idle_hook_point, [reference_e_pos, total_length, this] {
            if (marlin_server::get_response_from_phase(Phase::purge) == Response::Abort) {
                if (confirm_abort(Phase::purge)) {
                    // Interrupt the purging
                    planner.quick_stop();
                }
            }
            if (!warn_active) { // To avoid drawing over the warning
                struct NozzleCleaningFailedProgressData data {
                    .progress_0_255 = static_cast<uint8_t>(std::min(255.f, fabs(static_cast<float>(marlin_vars().native_pos[MARLIN_VAR_INDEX_E] - reference_e_pos) / static_cast<float>(total_length)) * 255))
                };
                fsm_change(Phase::purge, fsm::serialize_data(data));
            }
        });
        mapi::extruder_move(purge_length, ADVANCED_PAUSE_PURGE_FEEDRATE);
        planner.synchronize();
        return !planner.draining();
    }

    void autoretract() {
        fsm_change(Phase::autoretract);
        auto progress_callback = [](float progress) {
            struct NozzleCleaningFailedProgressData data {
                .progress_0_255 = static_cast<uint8_t>((progress / 100) * 255)
            };
            fsm_change(Phase::autoretract, fsm::serialize_data(data));
        };
        buddy::auto_retract().maybe_retract_from_nozzle(stdext::inplace_function<void(float)>(progress_callback));
        planner.synchronize();
    }

    [[nodiscard]] bool ask_user_to_clean_nozzle() {
        while (true) {
            fsm_change(Phase::remove_filament);
            const auto response = marlin_server::wait_for_response(Phase::remove_filament);
            switch (response) {
            case Response::Done:
                return true;

            case Response::Abort:
                if (confirm_abort(Phase::remove_filament)) {
                    return false;
                } else {
                    continue;
                }
            default:
                BUDDY_UNREACHABLE();
            }
        }
    }
#endif

    [[nodiscard]] bool confirm_abort(Phase to_restore) {
        warn_active = true;
        fsm_change(Phase::warn_abort);

        const auto response = marlin_server::wait_for_response(Phase::warn_abort);
        switch (response) {
        case Response::Yes:
            // Do not restore the original phase, we are aborting, this would cause unnecessary redraw
            return true;
        case Response::No:
            fsm_change(to_restore);
            warn_active = false;
            return false;
        default:
            BUDDY_UNREACHABLE();
        }
    }
};

} // namespace

Result nozzle_cleaning_failed_wizard::run_wizard() {
    NozzleCleaningFailedWizard wizard;
    return wizard.run();
}
