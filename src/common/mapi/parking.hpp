#pragma once

#include <core/types.h>
#include <utils/enum_array.hpp>
#include <variant>

#include <option/has_nozzle_cleaner.h>
#include <option/has_wastebin.h>
#include <buddy/unreachable.hpp>

namespace mapi {

enum class ZAction : uint16_t {
    move_to_at_least,
    absolute_move,
    relative_move,
    relative_move_skip_xy, // TODO not implemented
    no_move,
    _cnt
};

enum class ParkPosition : uint8_t {
    park,
    purge,
    load,
    _cnt,
};

/*
This variant version of xyz_pos_t tries to solve the problem of xyz_t having overloaded meaning,
1. absolute position (position in a physical sense) - cannot be NaN
2. position to change to (can be NaN -> it means no change on that specific axis)

This class serves as representation of a position with higher level of abstraction,
it forces you to handle the ParkingPosition::Unchanged value and deal with the real meaning of this class.

It's intended use it to produce xyz_pos_t instance at the end of it's lifetime, having handled ParkingPosition::Unchanged values before that.
*/
struct ParkingPosition {
    // special marker indicating "leave the synchronized xyz_pos_t on that axis as is"
    struct Unchanged {
        constexpr auto operator<=>(const Unchanged &) const = default;
    };
    using Variant = std::variant<float, Unchanged>;

    static constexpr Variant unchanged = Unchanged {};
    Variant x, y, z;

    // Synchronizes this provided position and provides appropriate xyz_pos_t
    xyz_pos_t to_xyz_pos(const xyz_pos_t &pos) const;

    // Do not use if not necessary! This method currently works as a
    // bridge between unrefactored parts still using xyz_pos_t
    // Should not be needed upon more refactoring
    xyz_pos_t to_nan_xyz_pos() const;

    static ParkingPosition from_xyz_pos(const xyz_pos_t &pos);

    // Provide array-like access if needed
    inline Variant &operator[](size_t index) {
        switch (index) {
        case 0:
            return x;
        case 1:
            return y;
        case 2:
            return z;
        default:
            BUDDY_UNREACHABLE();
        }
    }
};

static constexpr EnumArray<ParkPosition, ParkingPosition, ParkPosition::_cnt> park_positions {
    { ParkPosition::park, ParkingPosition(XYZ_NOZZLE_PARK_POINT) },
#if HAS_WASTEBIN()
        { ParkPosition::purge, ParkingPosition { X_WASTEBIN_POINT, Y_WASTEBIN_POINT, Z_AXIS_LOAD_POS } },
#else
        { ParkPosition::purge, ParkingPosition { X_AXIS_LOAD_POS, Y_AXIS_LOAD_POS, Z_AXIS_LOAD_POS } },

#endif
        { ParkPosition::load, ParkingPosition { X_AXIS_LOAD_POS, Y_AXIS_LOAD_POS, Z_AXIS_LOAD_POS } },
};

#if HAS_NOZZLE_CLEANER()
void move_out_of_nozzle_cleaner_area();
#endif

void park_move_with_conditional_home(const ParkingPosition &park_position, ZAction z_action);

void park(const uint8_t z_action, const xyz_pos_t &park = { { XYZ_NOZZLE_PARK_POINT } });

} // namespace mapi
