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

#include "../inc/MarlinConfig.h"

#include "nozzle.h"

Nozzle nozzle;

#include "../Marlin.h"
#include "../module/motion.h"

  void Nozzle::park(const uint8_t z_action, const xyz_pos_t &park/*={{XYZ_NOZZLE_PARK_POINT}}*/) {
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
      static constexpr xyz_pos_t default_park{{XYZ_NOZZLE_PARK_POINT}};
      if(park == default_park) {
        xy_pos_t pre_park{{{X_NOZZLE_PRE_PARK_POINT, std::min(current_position.y, static_cast<float>(Y_WASTEBIN_SAFE_POINT))}}};
        do_blocking_move_around_nozzle_cleaner_to_xy(pre_park, fr_xy);
      }
    #endif
    do_blocking_move_around_nozzle_cleaner_to_xy(park, fr_xy);
    report_current_position();
  }
