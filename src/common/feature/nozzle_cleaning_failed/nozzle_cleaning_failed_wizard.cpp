#include "nozzle_cleaning_failed_wizard.hpp"
#include <buddy/unreachable.hpp>
#include <feature/auto_retract/auto_retract.hpp>
#include <gcode/temperature/M104_M109.hpp>
#include <mapi/motion.hpp>
#include <fsm/nozzle_cleaning_phases.hpp>

#include <Marlin/src/Marlin.h>
#include <common/marlin_server.hpp>
#include <gcode/gcode.h>

using namespace marlin_server;
using namespace nozzle_cleaning_failed_wizard;
using Phase = PhaseNozzleCleaningFailed;

namespace {
enum class InnerResult {
    abort, // abort print
    ignore, // continute without retrying
    retry, // retry nozzle cleaning without purge
    ok,
};

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

    InnerResult run() {
        const float temp_before = Temperature::degTargetHotend(active_extruder);
        const float target_temp = config_store().get_filament_type(active_extruder).parameters().nozzle_temperature;
        InnerResult result;
        // Warn user that cleaning failed
        result = cleaning_failed();
        if (result != InnerResult::ok) {
            return result;
        }
        // Recommend purge
        result = recommend_purge();
        if (result != InnerResult::ok) {
            return result;
        }
        // Wait for temp
        result = wait_temp(target_temp);
        if (result != InnerResult::ok) {
            return result;
        }
        // Purge
        result = purge();
        if (result != InnerResult::ok) {
            return result;
        }
        // Autoretract
        result = autoretract();
        if (result != InnerResult::ok) {
            return result;
        }
        // Remove filament
        result = remove_filament();
        if (result != InnerResult::ok) {
            return result;
        }
        // Restore temp
        result = wait_temp(temp_before);
        if (result != InnerResult::ok) {
            return result;
        }
        return InnerResult::retry; // The wizard is done -> retry nozzle cleaning
    }

    InnerResult cleaning_failed() {
        while (true) {
            fsm_change(Phase::cleaning_failed);
            const auto response = marlin_server::wait_for_response(Phase::cleaning_failed);
            switch (response) {
            case Response::Retry:
#if HAS_NOZZLE_CLEANER() || !HAS_AUTO_RETRACT()
                // I case we have nozzle cleaner OR we are missing autoretract, no sense in normal purging so return
                return InnerResult::retry;
#else
                return InnerResult::ok;
#endif
            case Response::Ignore:
                return InnerResult::ignore;
            case Response::Abort:
                if (warn_abort(Phase::cleaning_failed)) {
                    return InnerResult::abort;
                } else {
                    continue;
                }
            default:
                BUDDY_UNREACHABLE();
            }
        }
    }

    InnerResult recommend_purge() {
        fsm_change(Phase::recommend_purge);
        const auto response = marlin_server::wait_for_response(Phase::recommend_purge);
        switch (response) {
        case Response::Yes:
            return InnerResult::ok;
        case Response::No:
            return InnerResult::retry;
        default:
            BUDDY_UNREACHABLE();
        }
    }

    InnerResult wait_temp(float target_temp) {
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
                warn_abort(Phase::wait_temp);
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
        if (planner.draining()) {
            return InnerResult::abort;
        }
        return InnerResult::ok;
    }

    InnerResult purge() {
        fsm_change(Phase::purge);
        constexpr int16_t purge_length = 20;

        const int16_t total_length = buddy::auto_retract().retracted_distance() + purge_length; // We need to ensure we purge even if the value is invalid (nullopt)
        int16_t reference_e_pos = marlin_vars().native_pos[MARLIN_VAR_INDEX_E];

        CallbackHookGuard subscriber(marlin_server::idle_hook_point, [reference_e_pos, total_length, this] {
            if (marlin_server::get_response_from_phase(Phase::purge) == Response::Abort) {
                warn_abort(Phase::purge);
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
        if (planner.draining()) {
            return InnerResult::abort;
        }
        return InnerResult::ok;
    }

    InnerResult autoretract() {
        fsm_change(Phase::autoretract);
        auto progress_callback = [](float progress) {
            struct NozzleCleaningFailedProgressData data {
                .progress_0_255 = static_cast<uint8_t>((progress / 100) * 255)
            };
            fsm_change(Phase::autoretract, fsm::serialize_data(data));
        };
        buddy::auto_retract().maybe_retract_from_nozzle(stdext::inplace_function<void(float)>(progress_callback));
        planner.synchronize();
        return InnerResult::ok;
    }

    InnerResult remove_filament() {
        while (true) {
            fsm_change(Phase::remove_filament);
            const auto response = marlin_server::wait_for_response(Phase::remove_filament);
            switch (response) {
            case Response::Done:
                // Set back target temp and wait
                return InnerResult::ok;
            case Response::Abort:
                if (warn_abort(Phase::remove_filament)) {
                    return InnerResult::abort;
                } else {
                    continue;
                }
            default:
                BUDDY_UNREACHABLE();
            }
        }
    }

    bool warn_abort(Phase to_restore) {
        warn_active = true;
        fsm_change(Phase::warn_abort);
        const auto response = marlin_server::wait_for_response(Phase::warn_abort);
        switch (response) {
        case Response::Yes:
            marlin_server::print_abort();
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
    switch (wizard.run()) {
    case InnerResult::abort:
        return Result::abort;
    case InnerResult::ignore:
        return Result::ignore;
    case InnerResult::retry:
        return Result::retry;
    case InnerResult::ok:
        BUDDY_UNREACHABLE();
    }
    BUDDY_UNREACHABLE();
}
