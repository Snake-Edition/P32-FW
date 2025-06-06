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

#include "../../inc/MarlinConfigPre.h"

#if EXTRUDERS

#include "../gcode.h"
#include "../../module/temperature.h"
#include "../../module/motion.h"
#include "../../module/planner.h"
#include "../../lcd/ultralcd.h"
#include "../../Marlin.h"

#if ENABLED(PRINTJOB_TIMER_AUTOSTART)
  #include "../../module/printcounter.h"
#endif

#if ENABLED(SINGLENOZZLE)
  #include "../../module/tool_change.h"
#endif

#include "marlin_server.hpp"

/** \addtogroup G-Codes
 * @{
 */

/**
 *### M104: Set hot end temperature <a href="https://reprap.org/wiki/G-code#M104:_Set_Extruder_Temperature">M104: Set Extruder Temperature</a>
 *
 *#### Usage
 *
 *    M104 [ S | D | T ]
 *
 * #### Parameters
 *
 * - `S` - Set temperature
 * - `D` - Display temperature (otherwise actual temp will be displayed)
 * - `T` - Tool
 */
void GcodeSuite::M104() {

  if (DEBUGGING(DRYRUN)) return;

  #if ENABLED(PRUSA_MMU2) // MMU2 doesn't handle different temps per slot, sayonara! (TODO?)
	  constexpr int8_t target_extruder = 0;
  #elif ENABLED(MIXING_EXTRUDER) && MIXING_VIRTUAL_TOOLS > 1
    constexpr int8_t target_extruder = 0;
  #else
    const int8_t target_extruder = get_target_extruder_from_command();
    if (target_extruder < 0) return;
  #endif

  if (parser.seenval('S')) {
    const int16_t temp = parser.value_celsius();
    #if ENABLED(SINGLENOZZLE)
      singlenozzle_temp[target_extruder] = temp;
      if (target_extruder != active_extruder) return;
    #endif
    thermalManager.setTargetHotend(temp, target_extruder);

    #if ENABLED(DUAL_X_CARRIAGE)
      if (dxc_is_duplicating() && target_extruder == 0)
        thermalManager.setTargetHotend(temp ? temp + duplicate_extruder_temp_offset : 0, 1);
    #endif

    #if ENABLED(PRINTJOB_TIMER_AUTOSTART)
      /*
       * Stop the timer at the end of print. Start is managed by 'heat and wait' M109.
       * We use half EXTRUDE_MINTEMP here to allow nozzles to be put into hot
       * standby mode, for instance in a dual extruder setup, without affecting
       * the running print timer.
       */
      if (temp <= (EXTRUDE_MINTEMP) / 2) {
        print_job_timer.pause();
        ui.reset_status();
      }
    #endif
  }

  marlin_server::set_temp_to_display(parser.seenval('D') ? parser.value_celsius() : thermalManager.degTargetHotend(target_extruder), target_extruder);

  #if ENABLED(AUTOTEMP)
    planner.autotemp_M104_M109();
  #endif
}

/**
 *### M109: Set hot end temperature and wait <a href="https://reprap.org/wiki/G-code#M109:_Set_Extruder_Temperature_and_Wait">M109: Set Extruder Temperature and Wait</a>
 *
 *#### Usage
 *
 *    M109 [ S | R | D | F | T ]
 *
 * #### Parameters
 *
 * - `S` - Wait for extruder(s) to reach temperature. Waits only when heating.
 * - `R` - Wait for extruder(s) to reach temperature. Waits when heating and cooling.
 * - `D` - Display temperature (otherwise actual temp will be displayed)
 * - `F` - Autotemp flag.
 * - `T` - Tool
 */
void GcodeSuite::M109() {

  if (DEBUGGING(DRYRUN)) return;

  #if ENABLED(PRUSA_MMU2) // MMU2 doesn't handle different temps per slot, sayonara! (TODO?)
	  constexpr int8_t target_extruder = 0;
  #elif ENABLED(MIXING_EXTRUDER) && MIXING_VIRTUAL_TOOLS > 1
    constexpr int8_t target_extruder = 0;
  #else
    const int8_t target_extruder = get_target_extruder_from_command();
    if (target_extruder < 0) return;
  #endif

  const bool no_wait_for_cooling = parser.seenval('S'),
             set_temp = no_wait_for_cooling || parser.seenval('R');
  if (set_temp) {
    const int16_t temp = parser.value_celsius();
    #if ENABLED(SINGLENOZZLE)
      singlenozzle_temp[target_extruder] = temp;
      if (target_extruder != active_extruder) return;
    #endif
    thermalManager.setTargetHotend(temp, target_extruder);

    #if ENABLED(DUAL_X_CARRIAGE)
      if (dxc_is_duplicating() && target_extruder == 0)
        thermalManager.setTargetHotend(temp ? temp + duplicate_extruder_temp_offset : 0, 1);
    #endif

    #if ENABLED(PRINTJOB_TIMER_AUTOSTART)
      // TODO: this doesn't work properly for multitool, temperature of last tool decides whenever printjob timer is started or not
      /*
       * Use half EXTRUDE_MINTEMP to allow nozzles to be put into hot
       * standby mode, (e.g., in a dual extruder setup) without affecting
       * the running print timer.
       */
      if (temp <= (EXTRUDE_MINTEMP) / 2) {
        print_job_timer.pause();
        ui.reset_status();
      }
      else
        print_job_timer.start();
    #endif

    #if HAS_DISPLAY
      if (thermalManager.isHeatingHotend(target_extruder) || !no_wait_for_cooling)
        thermalManager.set_heating_message(target_extruder);
    #endif
  }

  #if ENABLED(AUTOTEMP)
    planner.autotemp_M104_M109();
  #endif

  if (set_temp) {
    marlin_server::set_temp_to_display(parser.seenval('D') ? parser.value_celsius() : thermalManager.degTargetHotend(target_extruder), target_extruder);
    (void)thermalManager.wait_for_hotend(target_extruder, no_wait_for_cooling, parser.seen('C'));
  }
}

/** @}*/

#endif // EXTRUDERS
