/// \file
#pragma once

#include <limits>

/// Utility class that tracks "combos" - consecutive events where each two events are within a set time limit from each other
template <typename Time, typename Combo>
class ComboCounter {

public:
    explicit ComboCounter(Time max_event_interval)
        : max_event_interval_(max_event_interval) {}

    inline Combo current_combo() const {
        return current_combo_;
    }

    void step(Time now, bool is_event) {
        if (static_cast<Time>(now - last_event_time_) > max_event_interval_) {
            reset();
        }

        if (is_event) {
            last_event_time_ = now;
            if (current_combo_ < std::numeric_limits<Combo>::max()) {
                current_combo_++;
            }
        }
    }

    void reset() {
        // No need to reset last_event_time_, it would only set combo to 0 anyway
        current_combo_ = 0;
    }

private:
    Time max_event_interval_;
    Time last_event_time_ = 0;
    Combo current_combo_ = 0;
};
