#pragma once

#include <cstdint>
#include <bitset>

namespace buddy {

/// Not thread safe, use only from defaultTask
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

    void set_object_cancelled(ObjectID obj, bool set);

    ObjectID current_object() const;

    bool is_current_object_cancelled() const;

    void set_current_object(ObjectID obj);

    ObjectID object_count() const;

    /// Prints cancel object information into the serial line
    void report() const;

    /// Only for power panic pruposes
    State state() const;

    /// Only for power panic pruposes
    void set_state(const State &set);

    void reset();

private:
    State state_;
};

/// Not thread safe, use only from defaultTask
CancelObject &cancel_object();

} // namespace buddy
