#include "safety_timer.hpp"

#include <marlin_server.hpp>
#include <RAII.hpp>
#include <bsod/bsod.h>
#include <feature/host_actions.h>
#include <utils/progress.hpp>
#include <fsm/safety_timer_phases.hpp>

namespace {
void handle_resuming_abort() {
    // Abort right away if not printing
    if (marlin_server::is_printing()) {
        marlin_server::fsm_change(PhaseSafetyTimer::abort_confirm);
        switch (marlin_server::wait_for_response(PhaseSafetyTimer::abort_confirm)) {

        case Response::No:
            return;

        case Response::Abort:
            // Continue aborting
            break;

        default:
            bsod_unreachable();
        }
    }

    marlin_server::print_abort();
    marlin_server::quick_stop();
}
} // namespace

namespace buddy {

SafetyTimer &safety_timer() {
    static SafetyTimer instance;
    return instance;
}

void SafetyTimer::set_interval(Time set) {
    activity_timer_.set_interval(std::max<Time>(set, 3000));
}

void SafetyTimer::reset_norestore() {
    if (prevent_recursion_) {
        return;
    }

    activity_timer_.restart(ticks_ms());
}

void SafetyTimer::reset_restore_nonblocking() {
    if (prevent_recursion_) {
        return;
    }

    reset_norestore();

    marlin_server::clear_warning(WarningType::NozzleTimeout);
    marlin_server::clear_warning(WarningType::HeatersTimeout);

    if (state_ != State::active) {
        // Nothing to restore
        return;
    }

    // recursion_guard must be created AFTER calling reset_norestore, otherwise it would cancel it out
    AutoRestore recursion_guard { prevent_recursion_, true };

    // !!! Set BEFORE setting the target temperatures, otherwise we would get a recursion
    state_ = State::restoring;

    for (uint8_t hotend = 0; hotend < HOTENDS; hotend++) {
        assert(thermalManager.degTargetHotend(hotend) <= 0);

        const auto temp = nozzle_temperatures_to_restore_[hotend];
        thermalManager.setTargetHotend(temp, hotend);
        marlin_server::set_temp_to_display(temp, hotend);
    }
}

void SafetyTimer::reset_restore_blocking() {
    assert(!prevent_recursion_);

    reset_restore_nonblocking();

    // Prevent the timer from timing out during the heatup
    SafetyTimerBlocker timer_blocker;

    std::array<float, HOTENDS> start_temperatures;
    for (uint8_t hotend = 0; hotend < HOTENDS; hotend++) {
        start_temperatures[hotend] = Temperature::degHotend(hotend);
    }

    while (!planner.draining() && state() == State::restoring) {
        float min_progress = 1;
        for (uint8_t hotend = 0; hotend < HOTENDS; hotend++) {
            const float hotend_progress = to_normalized_progress(start_temperatures[hotend], Temperature::degTargetHotend(hotend), Temperature::degHotend(hotend));
            min_progress = std::min(min_progress, hotend_progress);
        }
        marlin_server::fsm_change(PhaseSafetyTimer::resuming, fsm::serialize_data<float>(min_progress * 100));

        if (marlin_server::get_response_from_phase(PhaseSafetyTimer::resuming) == Response::Abort) {
            handle_resuming_abort();
        }

        idle(true);
    }

    marlin_server::fsm_destroy(ClientFSM::SafetyTimer);
}

void SafetyTimer::trigger() {
    assert(!prevent_recursion_);
    AutoRestore recursion_guard { prevent_recursion_, true };

    if (blocker_count_ > 0) {
        // Never call this when a blocker is active
        bsod_unreachable();
    }

    // In case the trigger was called explicitly from somewhere
    activity_timer_.stop();

    bool has_anything_to_disable = false;
    for (uint8_t hotend = 0; hotend < HOTENDS; hotend++) {
        has_anything_to_disable |= (Temperature::degTargetHotend(hotend) > 0);
    }

    const bool should_activate = marlin_server::is_processing() || marlin_server::is_printing();

    // We are not somewhere that would need the temperatures to be restored -> disable all heaters and call it a day
    if (!should_activate) {
        state_ = State::idle;

#if PRINTER_IS_PRUSA_iX()
        // On iX, don't turn off the heatbed in Finished state. If the harvester
        // wouldn't harvest the print and the bed would cool down, it'd cause
        // the print to be detached and greatly increase the chance of
        // harvesting failure.
        bool disable_all = (marlin_vars().print_state != marlin_server::State::Finished);
#else
        constexpr bool disable_all = true;
#endif

        if (disable_all) {
            has_anything_to_disable |= (Temperature::degTargetBed() > 0);
        }

        if (has_anything_to_disable) {
#ifdef ACTION_ON_SAFETY_TIMER_EXPIRED
            host_action_safety_timer_expired();
#endif

            if (disable_all) {
                Temperature::disable_all_heaters();
                marlin_server::set_warning(WarningType::HeatersTimeout);
            } else {
                Temperature::disable_hotend();
                marlin_server::set_warning(WarningType::NozzleTimeout);
            }
        }
        return;
    }

    if (!has_anything_to_disable) {
        return;
    }

    if (state_ == State::active) {
        // We should NEVER get to the state where we have heaters on when the SafetyTimer is marked as active.
        // setTargetHotend HAS to reset the safety timer before changing target temps
        bsod_unreachable();
    }

    state_ = State::active;

    for (uint8_t hotend = 0; hotend < HOTENDS; hotend++) {
        nozzle_temperatures_to_restore_[hotend] = thermalManager.degTargetHotend(hotend);
    }

#ifdef ACTION_ON_SAFETY_TIMER_EXPIRED
    host_action_safety_timer_expired();
#endif

    Temperature::disable_hotend();
    marlin_server::set_warning(WarningType::NozzleTimeout);
}

void SafetyTimer::step() {
    assert(!prevent_recursion_);

    const auto now = ticks_ms();

    // Doing any motor movement resets the activity timer
    if (planner.busy()) {
        activity_timer_.restart(now);
    }

    // Note: If someone re-enables heaters, the SafetyTimer should get reset, so we don't need to disable the heaters continuously
    if (activity_timer_.check(now) && blocker_count_ == 0) {
        trigger();
    }

    if (state_ == State::restoring && Temperature::are_hotend_temperatures_reached()) {
        state_ = State::idle;
    }
}

SafetyTimerBlocker::SafetyTimerBlocker() {
    auto &st = safety_timer();
    st.blocker_count_++;

    // Ensure that the SafetyTimer is not already active
    st.reset_restore_nonblocking();

    // Check for overflows
    assert(st.blocker_count_ > 0);
}

SafetyTimerBlocker::~SafetyTimerBlocker() {
    auto &st = safety_timer();

    // One last kick when we're exiting - SafetyTimerBlocker is considered an activity
    // This is very important. If the timer times out inside a blocker, it needs to get restarted,
    // otherwise it would get stuck in a finished state
    st.reset_restore_nonblocking();

    assert(st.blocker_count_ > 0);
    st.blocker_count_--;
}

} // namespace buddy
