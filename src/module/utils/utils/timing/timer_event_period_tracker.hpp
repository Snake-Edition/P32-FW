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
    /// Handle capture and/or timer overflow events.
    /// When using interrupts, it might not always be clear which event happened sooner, so this function has logic to cover that.
    void handle_multi_event(uint16_t timer_value, bool was_capture, bool was_overflow);

    /// Record an event. Typically called from the timer IRQ
    /// \param timer_value value of the timer at the time of the event
    /// !!! We recommend using handle_multi_event
    inline void handle_event(uint16_t timer_value) {
        handle_multi_event(timer_value, true, false);
    }

    /// Process timer overflow. Typically called from the timer overflow IRQ
    /// !!! We recommend using handle_multi_event
    inline void handle_timer_overflow() {
        handle_multi_event(0, false, true);
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
