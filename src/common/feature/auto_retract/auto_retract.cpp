#include "auto_retract.hpp"

#include <marlin_vars.hpp>
#include <config_store/store_instance.hpp>
#include <feature/ramming/standard_ramming_sequence.hpp>
#include <module/planner.h>
#include <RAII.hpp>
#include <filament_sensors_handler.hpp>
#include <logging/log.hpp>
#include <feature/print_status_message/print_status_message_guard.hpp>
#include <marlin_server.hpp>
#include <feature/prusa/e-stall_detector.h>
#include <mapi/motion.hpp>

#include <option/has_mmu2.h>
#if HAS_MMU2()
    #include <Marlin/src/feature/prusa/MMU2/mmu2_mk4.h>
#endif

// Auto retract functionality collides with the nozzle cleaner
#include <option/has_nozzle_cleaner.h>
static_assert(!HAS_NOZZLE_CLEANER());

LOG_COMPONENT_REF(MarlinServer);

using namespace buddy;

AutoRetract &buddy::auto_retract() {
    static AutoRetract instance;
    return instance;
}

AutoRetract::AutoRetract() {
    for (uint8_t i = 0; i < HOTENDS; i++) {
        retracted_hotends_bitset_.set(i, config_store().get_filament_retracted_distance(i).value_or(0.0f) > 0.0f);
    }
}

bool AutoRetract::will_deretract() const {
    const auto hotend = marlin_vars().active_hotend_id();
    return will_deretract(hotend);
}

bool AutoRetract::will_deretract(uint8_t hotend) const {
    return retracted_hotends_bitset_.test(hotend);
}

bool AutoRetract::is_safely_retracted_for_unload(uint8_t hotend) const {
    const auto dist = config_store().get_filament_retracted_distance(hotend);
    return dist.has_value() && dist.value() >= minimum_auto_retract_distance;
}

bool AutoRetract::is_safely_retracted_for_unload() const {
    const auto hotend = marlin_vars().active_hotend_id();
    return is_safely_retracted_for_unload(hotend);
}

std::optional<float> AutoRetract::retracted_distance() const {
    const auto hotend = marlin_vars().active_hotend_id();
    return config_store().get_filament_retracted_distance(hotend);
}

void AutoRetract::set_retracted_distance(uint8_t hotend, std::optional<float> dist) {
    retracted_hotends_bitset_.set(hotend, dist.value_or(0.0f) > 0.0f);
    config_store().set_filament_retracted_distance(hotend, dist);
}

void AutoRetract::maybe_retract_from_nozzle(const ProgressCallback &progress_callback) {
    if (!can_perform_action()) {
        return;
    }

    const auto hotend = marlin_vars().active_hotend_id();

    // Is already retracted -> exit
    if (is_safely_retracted_for_unload(hotend)) {
        return;
    }

    // Do not auto retract specific filaments (for example TPU might get tangled up in the extruder - BFW-6953)
    if (config_store().get_filament_type(hotend).parameters().do_not_auto_retract) {
        return;
    }

    // Finish all pending movements so that the progress reporting is nice
    planner.synchronize();

    PrintStatusMessageGuard psm_guard;
    psm_guard.update<PrintStatusMessage::Type::auto_retracting>({});

    const auto orig_e_position = planner.get_position_msteps().e;
    const auto orig_current_e_position = current_position.e;

    const auto &sequence = standard_ramming_sequence(StandardRammingSequence::auto_retract, hotend);
    {
        // No estall detection during the ramming; we may do so too fast sometimes
        // to the point where the motor skips, but we don't care, as it doesn't
        // damage the print.
        BlockEStallDetection estall_blocker;

        struct {
            uint32_t start_time;
            float progress_coef;
            const ProgressCallback &progress_callback;
        } progress_data {
            ticks_ms(),
            100.0f / sequence.duration_estimate_ms(),
            progress_callback
        };

        CallbackHookGuard progress_guard(marlin_server::idle_hook_point, [&] {
            const float progress = std::min((ticks_ms() - progress_data.start_time) * progress_data.progress_coef, 100.0f);
            psm_guard.update<PrintStatusMessage::Type::auto_retracting>({ progress });
            if (progress_data.progress_callback) {
                progress_data.progress_callback(progress);
            }
        });
        sequence.execute();
    }

    // "Fake" original extruder position - we are interrupting various movements by this function,
    // firmware gets very confused if the current position changes while it is planning a move
    planner.set_e_position_mm(orig_e_position);
    current_position.e = orig_current_e_position;

    assert(sequence.retracted_distance() >= minimum_auto_retract_distance);
    set_retracted_distance(hotend, sequence.retracted_distance());
}

void AutoRetract::maybe_deretract_to_nozzle() {
    // Prevent deretract nesting
    if (is_checking_deretract_) {
        return;
    }
    AutoRestore ar(is_checking_deretract_, true);

    const auto hotend = marlin_vars().active_hotend_id();

    // Is not retracted -> exit
    if (!will_deretract(hotend)) {
        return;
    }

    if (!can_perform_action()) {
        log_error(MarlinServer, "auto_retract: Cannot perform deretract");
        return;
    }

    const auto orig_e_position = planner.get_position_msteps().e;
    const auto orig_current_e_position = current_position.e;

    {
        // No estall detection during the ramming; we may do so too fast sometimes
        // to the point where the motor skips, but we don't care, as it doesn't
        // damage the print.
        BlockEStallDetection estall_blocker;
        mapi::extruder_move(retracted_distance().value_or(0.0f), FILAMENT_CHANGE_FAST_LOAD_FEEDRATE);
        planner.synchronize();
    }

    // "Fake" original extruder position - we are interrupting various movements by this function,
    // firmware gets very confused if the current position changes while it is planning a move
    planner.set_e_position_mm(orig_e_position);
    current_position.e = orig_current_e_position;

    set_retracted_distance(hotend, std::nullopt);
}

bool AutoRetract::can_perform_action() const {
    if (thermalManager.tooColdToExtrude(active_extruder)) {
        return false;
    }

    if (planner.draining()) {
        return false;
    }

    return true;
}
