#include "timer_event_period_tracker.hpp"
#include <cstdlib>

void TimerEventPeriodTracker::handle_multi_event(uint16_t timer_value, bool was_capture, bool was_overflow) {
    // Disregard overflows if we've exceeded the limit
    was_overflow &= (timer_overflows_since_last_event_ != max_timer_overflows);

    // If overflow and capture happened at the same time, heuristically determine which one happened sooner - otherwise we get invalid data
    // Timer value was in the lower half of the maximum -> the overflow likely happened before the event
    bool was_overflow_before_capture = was_overflow && was_capture && (timer_value < 0x800);

    // If we would come up with a negative period, assume there was an overflow before capture
    // This could happen if the heurstic fails:
    // OVERFLOW + CAPTURE, timer_value = 100.
    // Heuristic assumes capture happened after overflow, but it was actually before.
    // Then we get CAPTURE, timer_value = 80 and here we go.
    // This doesn't fix the issue fully, but at least prevents from returning garbage data.
    // And this issue shouldn't happen anyway if we don't do handle_multi_event too long after.
    if (was_capture && last_event_timer_val_ >= timer_value && timer_overflows_since_last_event_ == 0) {
        was_overflow_before_capture = true;
    }

    if (was_overflow_before_capture) {
        timer_overflows_since_last_event_++;
    }

    if (was_capture) {
        previous_event_timer_val_ = last_event_timer_val_;
        last_event_timer_val_ = timer_value;

        timer_overflows_between_last_and_previous_event_ = timer_overflows_since_last_event_;
        timer_overflows_since_last_event_ = 0;
    }

    if (was_overflow && !was_overflow_before_capture) {
        timer_overflows_since_last_event_++;
    }
}

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
