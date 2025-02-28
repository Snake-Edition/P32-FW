#pragma once

#include <cstdint>
#include <bitset>

#include <freertos/mutex.hpp>

namespace buddy {

/// Thread-safe for reading. Non-const functions should only be called from the marlin thread
class CancelObject {

public:
    using ObjectID = int16_t;

    /// How many objects we are able to track
    static constexpr ObjectID max_object_count = 64;

    struct State {
        /// Set of objects that are marked as cancelled
        std::bitset<max_object_count> cancelled_objects {};

        /// Count of objects that can be skipped
        ObjectID object_count = 0;

        /// Variable whose value changes with each change to object count/cancelled object bitset
        uint32_t objects_revision = 0;

        // ID of the currently printed object
        ObjectID current_object = 0;

        /// Whether we are currently in gcode skipping mode
        bool current_object_cancelled = false;
    };

public:
    uint64_t cancelled_objects_mask() const;

    /// \returns whether the object can be cancelled (the record of cancelling it fits in the internal structures)
    bool is_object_cancellable(ObjectID obj) const;

    bool is_object_cancelled(ObjectID obj) const;

    /// !!! Only to be called from the marlin thread
    void set_object_cancelled(ObjectID obj, bool set);

    ObjectID current_object() const;

    bool is_current_object_cancelled() const;

    /// !!! Only to be called from the marlin thread
    void set_current_object(ObjectID obj);

    ObjectID object_count() const;

    /// \returns value that changes each time object count/cancel mask is changed
    uint32_t objects_revision() const;

    /// Prints cancel object information into the serial line
    /// !!! Only to be called from the marlin thread
    void report() const;

    /// Only for power panic pruposes
    State state() const;

    /// Only for power panic pruposes
    /// !!! Only to be called from the marlin thread
    void set_state(const State &set);

    void reset();

private:
    bool is_object_cancelled_nolock(ObjectID obj) const;

private:
    State state_;

    mutable freertos::Mutex mutex_;
};

/// Thread-safe for reading. Non-const functions should only be called from the marlin thread
CancelObject &cancel_object();

} // namespace buddy
