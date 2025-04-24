#include "timer_event_period_tracker.hpp"

uint32_t TimerEventPeriodTracker::period_unsafe() const {
    // Check if we haven't got an event for a long enough time
    if (timer_overflows_since_last_event_ == max_timer_overflows) {
        return invalid_period;
    }

    // Check if the last two edges weren't too far apart
    if (timer_overflows_between_last_and_previous_event_ == max_timer_overflows) {
        return invalid_period;
    }

    return
        // We need to up-cast to uint32_t so that we get corrent "sign"
        uint32_t(last_event_timer_val_) - uint32_t(previous_event_timer_val_)

        // Timer is 16-bit, so shift left by 16
        + (uint32_t(timer_overflows_between_last_and_previous_event_) << 16);
}
