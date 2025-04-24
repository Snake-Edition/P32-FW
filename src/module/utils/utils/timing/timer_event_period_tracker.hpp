/// @file
#pragma once

#include <cstdint>

/// Class that tracks period (in timer ticks) between two events.
/// The timer is expected to be 16-bit, counting up to 65535 and then overflowing
class TimerEventPeriodTracker {

public:
    static constexpr uint8_t max_timer_overflows = 16;

    static constexpr uint32_t invalid_period = uint32_t(-1);

public:
    /// Record an event. Typically called from the timer IRQ
    /// \param timer_value value of the timer at the time of the event
    inline void handle_event(uint16_t timer_value) {
        previous_event_timer_val_ = last_event_timer_val_;
        last_event_timer_val_ = timer_value;

        timer_overflows_between_last_and_previous_event_ = timer_overflows_since_last_event_;
        timer_overflows_since_last_event_ = 0;
    }

    /// Process timer overflow. Typically called from the timer overflow IRQ
    inline void handle_timer_overflow() {
        // Prevent overflowing the generation difference counter
        if (timer_overflows_since_last_event_ != max_timer_overflows) {
            timer_overflows_since_last_event_++;
        }
    }

    /// \returns period between the last two recorded events (in timer ticks), or \p invalid_period if there is not enough data
    /// !!! There is a possibility of race condition when the timer IRQs happen during calling this function. Make sure you have disabled them for this call.
    uint32_t period_unsafe() const;

private:
    uint16_t last_event_timer_val_ = 0;
    uint16_t previous_event_timer_val_ = 0;

    // Initialize to max_timer_overflows to indicate an invalid value
    uint8_t timer_overflows_since_last_event_ = max_timer_overflows;
    uint8_t timer_overflows_between_last_and_previous_event_ = max_timer_overflows;
};
