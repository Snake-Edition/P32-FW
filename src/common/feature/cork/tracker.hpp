#pragma once

#include <freertos/mutex.hpp>

#include <array>
#include <optional>
#include <cstdint>

namespace buddy::cork {

/// A "backend" for tracking waiting for gcodes to finish.
///
/// The intended use is:
/// * A thread submits some gcodes.
/// * Now it wants to know when they are done with (either completed or canceled in some way).
/// * It allocates a waiting cookie from here, represented by the Waiter object.
/// * Creates another gcode (M9933 C<cookie-number>) and submits that.
/// * Repeatedly polls the held handle object until it is done.
///
/// Notes:
/// * There's a limited amount of slots (currently 2, it is expected each of
///   Connect and GUI may use one each).
/// * Discarding the handle cancels the wait (eg. it won't hold a slot indefinitely, RAII works here).
/// * Marlin marks the cork as done when the submited gcode is executed and
///   also when it empties the gcode queue.
/// * It acts "defensively" from the gcode side. As a sure can incorporate a
///   "rogue" instance of the gcode, we simply ignore any requests for cookies
///   that we don't have. That makes using rogue gcodes impractical (most of
///   the time, they'd just have no effect) instead of causing any disruption.
class Tracker {
public:
    using Cookie = uint16_t;

private:
    static constexpr size_t slot_cnt = 2;
    // With two slots, we fit it into one bit.
    static constexpr Cookie slot_mask = 1;
    static_assert(slot_cnt <= slot_mask + 1);

    struct Slot {
        bool done = false;
        Cookie cookie;
    };
    std::array<std::optional<Slot>, slot_cnt> slots;
    freertos::Mutex mutex;

    Tracker(const Tracker &other) = delete;
    Tracker(Tracker &&other) = delete;
    Tracker operator=(const Tracker &other) = delete;
    Tracker operator=(Tracker &&other) = delete;

    static size_t idx(Cookie cookie) { return cookie & slot_mask; }
    std::optional<Slot> *get_slot(Cookie cookie);

public:
    Tracker() = default;

    /// A RAII guard when holding a slot and waiting for it.
    class CorkHandle {
    private:
        friend class Tracker;
        Cookie cookie;
        Tracker *owner;

        CorkHandle(Cookie cookie, Tracker *owner)
            : cookie(cookie)
            , owner(owner) {}
        void destroy();

    public:
        CorkHandle(CorkHandle &&other);
        CorkHandle &operator=(CorkHandle &&other);
        ~CorkHandle();

        /// Check if it is already done.
        ///
        /// Once this returns `true`, the slot is *consumed* and shall not be called again.
        bool is_done_and_consumed();

        /// Returns the numerical cookie identifying the slot.
        ///
        /// Shall be used as part of the gcode to be submitted.
        Cookie get_cookie() const { return cookie; }

        /// Formats the corresponding gcode.
        ///
        /// Puts M9933 C<cookie> in there. Size is with a bit of a headroom.
        std::array<char, 20> get_gcode() const;
    };

    /// Requests a slot to wait for.
    ///
    /// May return nullopt in case all slots are full.
    std::optional<CorkHandle> new_cork();
    /// Marks *all* slots as done (used when emptying the gcode queue).
    void clear();
    /// Marks specific slot (identified by the cookie) as done.
    void mark_done(Cookie cookie);
};

extern Tracker tracker;

}; // namespace buddy::cork
