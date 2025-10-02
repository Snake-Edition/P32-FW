/// @file
#pragma once

#include <cstdint>
#include <array>

#include <utils/uncopyable.hpp>
#include <utils/timing/timer.hpp>
#include <option/has_human_interactions.h>
#include <inc/MarlinConfig.h>

namespace buddy {

/// Class responsible for disabling the heaters when the printer is inactive for some amount of time
class SafetyTimer {
    friend SafetyTimer &safety_timer();
    friend class SafetyTimerBlocker;

public:
    using Time = uint32_t;

    using NozzleTargetTemperatures = std::array<int16_t, HOTENDS>;

    enum class State : uint8_t {
        /// Either not expired or expired without the ability to restore (= outside of print)
        idle,

        /// The safety timer is triggered and is holding the temperatures to be restored
        active,

        /// Restoring the temperatures back to the normal
        restoring,
    };

#if !HAS_HUMAN_INTERACTIONS()
    static constexpr Time default_interval = 10 * 60 * 1000;
#else
    static constexpr Time default_interval = 30 * 60 * 1000;
#endif

public:
    inline Time interval() const {
        return activity_timer_.interval();
    }

    void set_interval(Time set);

public:
    inline State state() const {
        return state_;
    }

    inline bool is_active() const {
        return state_ == State::active;
    }

    const NozzleTargetTemperatures &nozzle_temperatures_to_restore() const {
        assert(is_active());
        return nozzle_temperatures_to_restore_;
    }

public:
    /// Restarts the inactivity timer, but if the timer is already triggered, it will NOT restore the temperatures
    void reset_norestore();

    /// RESTART the inactivity timer and RESTORE the temperatures (if the timer was holding)
    /// Restoring the temperatures would be NONBLOCKING
    /// The target temperatures will be set, but the function will not wait for them to be reached
    void reset_restore_nonblocking();

    /// RESTARTS the inactivity timer and BLOCKINGLY RESTORES the temperatures (if the timer was holding)
    void reset_restore_blocking();

    /// Trigger safety timer, disable all heaters and whatnot
    /// Will throw a BSOD if a SafetyTimerBlocker is active
    void trigger();

public:
    /// Needs to be called periodically to make things work
    void step();

private:
    SafetyTimer() = default;

private:
    State state_ = State::idle;

    /// Number of currently active SafetyTimerBlocker guards
    uint8_t blocker_count_ = 0;

    /// Timestamp of the last activity
    utils::Timer<Time> activity_timer_ { default_interval };

    /// Target nozzle temperatures to restore
    NozzleTargetTemperatures nozzle_temperatures_to_restore_;

    /// Recursion prevention mechanism
    bool prevent_recursion_ = false;
};

/// Guard marking a section where safety timer will not expire
/// The guard does NOT call \p restore_blocking()
class SafetyTimerBlocker : public Uncopyable {

public:
    SafetyTimerBlocker();
    ~SafetyTimerBlocker();
};

/// @returns instance to the SafetyTimer
/// !!! ONLY to be accessed from the defaultTask
SafetyTimer &safety_timer();

} // namespace buddy
