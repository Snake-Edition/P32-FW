#include "cancel_object.hpp"

#include <algorithm>

#include <gcode/gcode.h>

using namespace buddy;

CancelObject &buddy::cancel_object() {
    static CancelObject instance;
    return instance;
}

uint64_t CancelObject::cancelled_objects_mask() const {
    static_assert(max_object_count <= 64);
    static_assert(std::is_same_v<uint64_t, unsigned long long>);
    return state_.cancelled_objects.to_ullong();
}

bool CancelObject::is_object_cancellable(ObjectID obj) const {
    return (obj >= 0) && (obj <= max_object_count);
}

bool CancelObject::is_object_cancelled(ObjectID obj) const {
    return is_object_cancellable(obj) && state_.cancelled_objects.test(obj);
}

void CancelObject::set_object_cancelled(ObjectID obj, bool set) {
    if (!is_object_cancellable(obj)) {
        return;
    }

    state_.cancelled_objects.set(obj, set);

    if (obj == state_.current_object) {
        state_.current_object_cancelled = set;
    }
}

bool CancelObject::is_current_object_cancelled() const {
    return state_.current_object_cancelled;
}

CancelObject::ObjectID CancelObject::current_object() const {
    return state_.current_object;
}

void CancelObject::set_current_object(ObjectID obj) {
    state_.object_count = std::max<ObjectID>(state_.object_count, obj + 1);
    state_.current_object = obj;
    state_.current_object_cancelled = is_object_cancelled(obj);
}

CancelObject::ObjectID CancelObject::object_count() const {
    return state_.object_count;
}

void CancelObject::report() const {
    if (state_.current_object >= 0) {
        SERIAL_ECHO_START();
        SERIAL_ECHOPGM("Active Object: ");
        SERIAL_ECHO(state_.current_object);
        SERIAL_EOL();
    }

    if (state_.cancelled_objects.any()) {
        SERIAL_ECHO_START();
        SERIAL_ECHOPGM("Canceled:");
        for (ObjectID i = 0; i < max_object_count; i++) {
            if (state_.cancelled_objects.test(i)) {
                SERIAL_CHAR(' ');
                SERIAL_ECHO(i);
            }
        }
        SERIAL_EOL();
    }
}

CancelObject::State CancelObject::state() const {
    return state_;
}

void CancelObject::set_state(const State &set) {
    state_ = set;
}

void CancelObject::reset() {
    set_state({});
}
