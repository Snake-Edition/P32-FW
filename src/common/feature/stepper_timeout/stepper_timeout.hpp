/// @file
#pragma once

#include <utils/uncopyable.hpp>
#include <utils/timing/timer.hpp>

namespace buddy {

/// Manager handling disabling steppers on inactivity
class StepperTimeoutManager : public Uncopyable {
    friend StepperTimeoutManager &stepper_timeout();

public:
    /// Returns timeout interval, in ms
    auto interval_ms() const {
        return timer_.interval();
    }

    /// Changes the interval after which stepeprs get automatically disabled
    void set_interval_ms(uint32_t set) {
        timer_.set_interval(set);
    }

public:
    /// Resets the inactivity timer
    void reset();

    /// Call periodically
    void step();

    void trigger();

private:
    StepperTimeoutManager();

private:
    utils::Timer<uint32_t> timer_;
};

StepperTimeoutManager &stepper_timeout();

} // namespace buddy
