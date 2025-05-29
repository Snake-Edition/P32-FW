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
/**
 * Does the extra parking moves except the last one to move in the correct
 * directions and avoid the nozzle cleaner.
 *
 *  RearArea = WasteArea + NoWasteLand         │ X_NOZZLE_PARK_POINT
 *                                             ▼
 * ┌────────────────NO─WASTE─LAND────────────────┬─────────WASTE─AREA─────────┐
 * │                                         .  .│                            │
 * │                                         .  .│                        * ◄─┼────X,Y_WASTEBIN_POINT
 * │                                         .  .│                            │
 * │                                         .  .│    ╲‾‾╲        ╱‾‾╱=>      │
 * │                                         ....│      ╲  ╲      |  |        │
 * │                                             │        ╲__╲    ╲ˍˍ╲=>      │
 * │                                             │                            │
 * ├─────────────────────────────────────────────┴────────────────────────────┤ ◄── Y_WASTEBIN_SAFE_POINT
 * │                                                                          │
 * │   _____  _____  _____ _   _ _______            _____  ______             │
 * │  |  __ \|  __ \|_   _| \ | |__   __|     /\   |  __ \|  ____|   /\       │
 * │  | |__) | |__) | | | |  \| |  | |       /  \  | |__) | |__     /  \      │
 * │  |  ___/|  _  /  | | | . ` |  | |      / /\ \ |  _  /|  __|   / /\ \     │
 * │  | |    | | \ \ _| |_| |\  |  | |     / ____ \| | \ \| |____ / ____ \    │        ▲
 * │  |_|    |_|  \_\_____|_| \_|  |_|    /_/    \_\_|  \_\______/_/    \_\   │        | Y
 * │                                                                          │        |
 * │                                                                          │        |      X
 * └──────────────────────────────────────────────────────────────────────────┘        O ──────►
 */
static void pre_park_move_pattern(const feedRate_t &feedrate, const xy_pos_t &destination) {
    static constexpr float x_border_point = X_NOZZLE_PARK_POINT + 1;

    const bool start_in_rear_area = current_position.y > Y_WASTEBIN_SAFE_POINT;
    const bool destination_in_rear_area = destination.y > Y_WASTEBIN_SAFE_POINT;

    const bool start_in_wastebin_area = start_in_rear_area && current_position.x > x_border_point;
    const bool destination_in_wastebin_area = destination_in_rear_area && destination.x > x_border_point;

    if (start_in_rear_area != destination_in_rear_area) { // One in the rear, other in print area
        if (start_in_wastebin_area || destination_in_wastebin_area) { // One in waste area, other in print area
            do_blocking_move_to_x(X_WASTEBIN_POINT, feedrate);
            do_blocking_move_to_y(destination.y, feedrate);
        } else { // One in no-waste land, other in print area
            if (start_in_rear_area) { // Start in no-waste land, end in print area
                do_blocking_move_to_y(destination.y, feedrate);
            } else { // Start in print area, end in no-waste land
                do_blocking_move_to_x(destination.x, feedrate);
            }
        }
    } else if (start_in_rear_area) { // Both in the rear area
        if (start_in_wastebin_area != destination_in_wastebin_area) { // One in waste area, other in no-waste land
            if (start_in_wastebin_area) { // Start in waste area, end in no-waste land
                do_blocking_move_to_x(X_WASTEBIN_POINT, feedrate);
                do_blocking_move_to_y(Y_WASTEBIN_SAFE_POINT, feedrate);
                do_blocking_move_to_x(destination.x, feedrate);
            } else { // Start in no-waste land, end in waste area
                do_blocking_move_to_y(Y_WASTEBIN_SAFE_POINT, feedrate);
                do_blocking_move_to_x(X_WASTEBIN_POINT, feedrate);
                do_blocking_move_to_y(destination.y, feedrate);
            }
        } else if (start_in_wastebin_area) { // Both in waste area
            do_blocking_move_to_x(destination.x, feedrate);
        } else { // Both in no-waste land
            do_blocking_move_to_y(Y_WASTEBIN_SAFE_POINT, feedrate);
            do_blocking_move_to_x(destination.x, feedrate);
        }
    } // Both in print area, no need to pre-park move
}

void move_out_of_nozzle_cleaner_area() {
    pre_park_move_pattern(NOZZLE_PARK_XY_FEEDRATE, { X_WASTEBIN_POINT, Y_WASTEBIN_SAFE_POINT });
}
#endif

void park(ZAction z_action, const ParkingPosition &parking_position) {
    static constexpr feedRate_t fr_xy = NOZZLE_PARK_XY_FEEDRATE, fr_z = NOZZLE_PARK_Z_FEEDRATE;

    if (parking_position.z != ParkingPosition::unchanged) {
        const float z = std::get<float>(parking_position.z);
        switch (z_action) {
        case mapi::ZAction::move_to_at_least: // Raise to at least the Z-park height
            do_blocking_move_to_z(_MAX(z, current_position.z), fr_z);
            break;
        case ZAction::absolute_move: // Go to Z-park height
            do_blocking_move_to_z(z, fr_z);
            break;
        case ZAction::relative_move: // Raise by Z-park height
            do_blocking_move_to_z(_MIN(current_position.z + z, Z_MAX_POS), fr_z);
            break;
        case ZAction::relative_move_skip_xy: // TODO: Not implemented yet
        case ZAction::no_move: /// No Z move, just XY park
            break;
        }
    }

    const xy_pos_t park_destination = parking_position.to_xyz_pos(current_position);
#if HAS_NOZZLE_CLEANER()
    pre_park_move_pattern(fr_xy, park_destination);
#endif
    do_blocking_move_to_xy(park_destination, fr_xy);

    report_current_position();
}

void home_if_needed_and_park(ZAction z_action, const ParkingPosition &parking_position) {
    const xyz_bool_t do_axis {
        .x = (parking_position.x != mapi::ParkingPosition::unchanged),
        .y = (parking_position.y != mapi::ParkingPosition::unchanged),
        .z = (parking_position.z != mapi::ParkingPosition::unchanged) && z_action == mapi::ZAction::absolute_move
    };
    if (axes_need_homing(X_AXIS | Y_AXIS | Z_AXIS)) {
        GcodeSuite::G28_no_parser(do_axis.x, do_axis.y, do_axis.z, { .only_if_needed = true, .z_raise = 3 });
    }

    park(z_action, parking_position);
}

} // namespace mapi
