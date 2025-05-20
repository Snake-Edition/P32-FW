/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2019 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "config_features.h"

#include <Marlin/src/gcode/gcode.h>
#include <Marlin/src/module/motion.h>
#include <common/gcode/gcode_parser.hpp>
#include <common/filament_sensors_handler.hpp>
#include <common/mapi/parking.hpp>

/** \addtogroup G-Codes
 * @{
 */

/**
 *### G27: Park the nozzle <a href="https://reprap.org/wiki/G-code#G27:_Park_toolhead">G27: Park toolhead</a>
 *
 *#### Usage
 *
 * G27 [ X | Y | Z | P ]
 *
 *#### Parameters
 *
 * - `X` - X park position
 * - `Y` - Y park position
 * - `Z` - Z park position
 * - `P` - Z action
 *   - `0` - (Default) Relative raise by NOZZLE_PARK_Z_RAISE_MIN before XY parking
 *   - `1` - Absolute move to NOZZLE_PARK_POINT.z before XY parking. This may move the nozzle down, so use with caution!
 *   - `2` - Relative raise by NOZZLE_PARK_POINT.z before XY parking.
 * - `W` - Use pre-defined park position. Usable only if X, Y and Z are not present as they override pre-defined behaviour.
 *   - `0` - Park
 *   - `1` - Purge
 *   - `2` - Load
 */
void GcodeSuite::G27() {
    GCodeParser2 parser;
    if (!parser.parse_marlin_command()) {
        return;
    }

    mapi::ZAction z_action { mapi::ZAction::move_to_at_least };
    parser.store_option('P', z_action, static_cast<uint32_t>(mapi::ZAction::_last) + 1);

    mapi::ParkingPosition parking_position;
    if (auto where_to_park = parser.option<mapi::ParkPosition>('W', mapi::ParkPosition::_cnt)) {
        parking_position = mapi::park_positions[*where_to_park];
    } else {
        auto parse_axis = [&parser](char letter, mapi::ParkingPosition::Variant &axis) {
            if (auto res = parser.option<float>(letter)) {
                axis = *res;
            }
        };

        parse_axis('X', parking_position.x);
        parse_axis('Y', parking_position.y);
        parse_axis('Z', parking_position.z);

        // If no axis has been specified (comparing against a position with all axes unchanged)
        if (parking_position == mapi::ParkingPosition {}) {
            parking_position = mapi::park_positions[mapi::ParkPosition::park];
        }
    }

    // If not homed and only Z clearance is requested, od just that, otherwise home and then park.
    if (axes_need_homing(X_AXIS | Y_AXIS | Z_AXIS)) {
        if (parking_position.x == mapi::ParkingPosition::unchanged && parking_position.y == mapi::ParkingPosition::unchanged && parking_position.z != mapi::ParkingPosition::unchanged && z_action == mapi::ZAction::move_to_at_least) {
            // Only Z axis is given in P=0 mode, do Z clearance
            do_z_clearance(std::get<float>(parking_position.z));
            return;
        }
    }

    mapi::home_and_park(z_action, parking_position);
}

/** @}*/
