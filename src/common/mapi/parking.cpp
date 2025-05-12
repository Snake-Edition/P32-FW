#include "parking.hpp"

#include <Marlin/src/gcode/gcode.h>
#include <Marlin/src/module/motion.h>

namespace mapi {

xyz_pos_t ParkingPosition::to_xyz_pos(const xyz_pos_t &pos) const {
    xyz_pos_t result {
        (x == unchanged) ? pos.x : std::get<float>(x),
        (y == unchanged) ? pos.y : std::get<float>(y),
        (z == unchanged) ? pos.z : std::get<float>(z)
    };

    if (std::isnan(result.x) || std::isnan(result.y) || std::isnan(result.z)) {
        bsod("Conversion to xyz_pos_t failed.");
    }
    return result;
}

xyz_pos_t ParkingPosition::to_nan_xyz_pos() const {
    return xyz_pos_t {
        (x == ParkingPosition::unchanged) ? std::numeric_limits<float>::quiet_NaN() : std::get<float>(x),
        (y == ParkingPosition::unchanged) ? std::numeric_limits<float>::quiet_NaN() : std::get<float>(y),
        (z == ParkingPosition::unchanged) ? std::numeric_limits<float>::quiet_NaN() : std::get<float>(z)
    };
}

ParkingPosition ParkingPosition::from_xyz_pos(const xyz_pos_t &pos) {
    return ParkingPosition {
        (std::isnan(pos.x) ? mapi::ParkingPosition::unchanged : pos.x),
        (std::isnan(pos.y) ? mapi::ParkingPosition::unchanged : pos.y),
        (std::isnan(pos.z) ? mapi::ParkingPosition::unchanged : pos.z),
    };
}

#if HAS_NOZZLE_CLEANER()
static void move_out_of_nozzle_cleaner_area_if_needed(const feedRate_t &feedrate, bool destination_in_wastebin_area) {
    const bool start_in_wastebin_area = current_position.x > (X_NOZZLE_PARK_POINT + 1) && current_position.y > Y_WASTEBIN_SAFE_POINT;

    // First move to the right edge (the safe way to cross over the v-blade)
    if (destination_in_wastebin_area || start_in_wastebin_area) {
        do_blocking_move_to_x(X_WASTEBIN_POINT, feedrate);
    }
    // If we are in the wastebin area, and need to move somewhere else OR we are somewhere else and need to move to the wastebin area, go through the safe point
    if (destination_in_wastebin_area != start_in_wastebin_area) {
        do_blocking_move_to_y(Y_WASTEBIN_SAFE_POINT, feedrate);
    }
}

void move_out_of_nozzle_cleaner_area() {
    move_out_of_nozzle_cleaner_area_if_needed(NOZZLE_PARK_XY_FEEDRATE, false);
}
#endif

void park_move_with_conditional_home(const ParkingPosition &park_position, ZAction z_action) {
    const xyz_bool_t do_axis {
        .x = (park_position.x != mapi::ParkingPosition::unchanged),
        .y = (park_position.y != mapi::ParkingPosition::unchanged),
        .z = (park_position.z != mapi::ParkingPosition::unchanged) && z_action == mapi::ZAction::absolute_move
    };
    if (axes_need_homing(X_AXIS | Y_AXIS | Z_AXIS)) {
        GcodeSuite::G28_no_parser(do_axis.x, do_axis.y, do_axis.z, { .only_if_needed = true, .z_raise = 3 });
    }
    park(std::to_underlying(z_action), park_position.to_xyz_pos(current_position));
}

/**
 * Simple helper function doing blocking move so that it avoids nozzle cleaner.
 * It should be used whenever there is a reasonably high probability of head
 * moving closely around nozzle cleaner
 */
static void move_around_nozzle_cleaner_to_xy(const xy_pos_t &destination, const feedRate_t &feedrate) {
#if HAS_NOZZLE_CLEANER()
    const bool destination_in_wastebin_area = destination.x > (X_NOZZLE_PARK_POINT + 1) && destination.y > Y_WASTEBIN_SAFE_POINT;
    move_out_of_nozzle_cleaner_area_if_needed(feedrate, destination_in_wastebin_area);
#endif

    do_blocking_move_to_xy(destination, feedrate);
}

void park(const uint8_t z_action, const xyz_pos_t &park /*={{XYZ_NOZZLE_PARK_POINT}}*/) {
    static constexpr feedRate_t fr_xy = NOZZLE_PARK_XY_FEEDRATE, fr_z = NOZZLE_PARK_Z_FEEDRATE;

    switch (z_action) {
    case 1: // Go to Z-park height
        do_blocking_move_to_z(park.z, fr_z);
        break;

    case 2: // Raise by Z-park height
        do_blocking_move_to_z(_MIN(current_position.z + park.z, Z_MAX_POS), fr_z);
        break;
    case 4: /// No Z move, just XY park
        break;
    default: // Raise to at least the Z-park height
        do_blocking_move_to_z(_MAX(park.z, current_position.z), fr_z);
    }

#ifdef X_NOZZLE_PRE_PARK_POINT
    static constexpr xyz_pos_t default_park { { XYZ_NOZZLE_PARK_POINT } };
    if (park == default_park) {
        xy_pos_t pre_park { { { X_NOZZLE_PRE_PARK_POINT, std::min(current_position.y, static_cast<float>(Y_WASTEBIN_SAFE_POINT)) } } };
        move_around_nozzle_cleaner_to_xy(pre_park, fr_xy);
    }
#endif
    move_around_nozzle_cleaner_to_xy(park, fr_xy);
    report_current_position();
}

} // namespace mapi
