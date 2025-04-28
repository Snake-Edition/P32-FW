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
#pragma once

#include "../inc/MarlinConfig.h"
#include <option/has_nozzle_cleaner.h>

/**
 * @brief Nozzle class
 *
 * @todo: Do not ignore the end.z value and allow XYZ movements
 */
class Nozzle {
  public:
    static void park(const uint8_t z_action, const xyz_pos_t &park={{XYZ_NOZZLE_PARK_POINT}}) __Os;
#if HAS_NOZZLE_CLEANER()
    static void move_out_of_nozzle_cleaner_area();
#endif
};

extern Nozzle nozzle;
