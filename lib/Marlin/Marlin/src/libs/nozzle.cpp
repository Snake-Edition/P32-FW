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

#if HAS_NOZZLE_CLEANER()
static void move_out_of_nozzle_cleaner_area_if_needed(const feedRate_t& feedrate, bool destination_in_wastebin_area) {
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

void Nozzle::move_out_of_nozzle_cleaner_area() {
  move_out_of_nozzle_cleaner_area_if_needed(NOZZLE_PARK_XY_FEEDRATE, false);
}
#endif

/**
 * Simple helper function doing blocking move so that it avoids nozzle cleaner.
 * It should be used whenever there is a reasonably high probability of head
 * moving closely around nozzle cleaner
 */
static void move_around_nozzle_cleaner_to_xy(const xy_pos_t& destination, const feedRate_t& feedrate) {
#if HAS_NOZZLE_CLEANER()
    const bool destination_in_wastebin_area = destination.x > (X_NOZZLE_PARK_POINT + 1) && destination.y > Y_WASTEBIN_SAFE_POINT;
    move_out_of_nozzle_cleaner_area_if_needed(feedrate, destination_in_wastebin_area);
#endif

    do_blocking_move_to_xy(destination, feedrate);
}

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
        move_around_nozzle_cleaner_to_xy(pre_park, fr_xy);
      }
    #endif
    move_around_nozzle_cleaner_to_xy(park, fr_xy);
    report_current_position();
  }
