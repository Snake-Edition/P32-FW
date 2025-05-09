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

#include "../../inc/MarlinConfig.h"

#if HAS_M206_COMMAND

#include "../gcode.h"
#include "../../module/motion.h"
#include "../../lcd/ultralcd.h"
#include "../../libs/buzzer.h"

/** \addtogroup G-Codes
 * @{
 */

/**
 *### M206: Get/Set Homing Offset (X Y Z) <a href="https://reprap.org/wiki/G-code#M206:_Offset_axes">M206: Offset axes</a>
 *
 *#### Usage
 *
 *    M206 [ X | Y | Z ]
 *
 *#### Parameters
 *
 * - `X` - X axis offset
 * - `Y` - Y axis offset
 * - `Z` - Z axis offset
 */
void GcodeSuite::M206() {
  LOOP_XYZ(i)
    if (parser.seen(axis_codes[i]))
      set_home_offset((AxisEnum)i, parser.value_linear_units());

  #if ENABLED(MORGAN_SCARA)
    if (parser.seen('T')) set_home_offset(A_AXIS, parser.value_float()); // Theta
    if (parser.seen('P')) set_home_offset(B_AXIS, parser.value_float()); // Psi
  #endif

  report_current_position();
}

/**
 *### M428: Set home offset based on distance <a href="https://reprap.org/wiki/G-code#M428:_Set_home_offset_based_on_distance">M428: Set home_offset based on distance</a>
 *
 * Set home_offset based on the distance between the current_position and the nearest "reference point."
 * If an axis is past center its endstop position is the reference-point. Otherwise it uses 0.
 * This allows the Z offset to be set near the bed when using a max endstop.
 *
 * M428 can't be used more than 2cm away from 0 or an endstop.
 *
 * Use M206 to set these values directly.
 *
 *#### Usage
 *
 *    M428 [ X | Y | Z ]
 *
 *#### Parameters
 *
 * - `X` - X axis offset distance
 * - `Y` - Y axis offset distance
 * - `Z` - Z axis offset distance
 */
void GcodeSuite::M428() {
  if (axis_unhomed_error()) return;

  xyz_float_t diff;
  LOOP_XYZ(i) {
    diff[i] = base_home_pos((AxisEnum)i) - current_position[i];
    if (!WITHIN(diff[i], -20, 20) && home_dir((AxisEnum)i) > 0)
      diff[i] = -current_position[i];
    if (!WITHIN(diff[i], -20, 20)) {
      SERIAL_ERROR_MSG(MSG_ERR_M428_TOO_FAR);
      LCD_ALERTMESSAGEPGM_P(PSTR("Err: Too far!"));
      BUZZ(200, 40);
      return;
    }
  }

  LOOP_XYZ(i) set_home_offset((AxisEnum)i, diff[i]);
  report_current_position();
  LCD_MESSAGEPGM(MSG_HOME_OFFSETS_APPLIED);
  BUZZ(100, 659);
  BUZZ(100, 698);
}

/** @}*/

#endif // HAS_M206_COMMAND
