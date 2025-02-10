#include "parking.hpp"

#include <Marlin/src/gcode/gcode.h>
#include <Marlin/src/module/motion.h>
#include <Marlin/src/libs/nozzle.h>

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

void park_move_with_conditional_home(const ParkingPosition &park_position, ZAction z_action) {
    const xyz_bool_t do_axis {
        .x = (park_position.x != mapi::ParkingPosition::unchanged),
        .y = (park_position.y != mapi::ParkingPosition::unchanged),
        .z = (park_position.z != mapi::ParkingPosition::unchanged) && z_action == mapi::ZAction::absolute_move
    };
    if (axes_need_homing(X_AXIS | Y_AXIS | Z_AXIS)) {
        GcodeSuite::G28_no_parser(do_axis.x, do_axis.y, do_axis.z, { .only_if_needed = true, .z_raise = 3 });
    }
    nozzle.park(std::to_underlying(z_action), park_position.to_xyz_pos(current_position));
}
} // namespace mapi
