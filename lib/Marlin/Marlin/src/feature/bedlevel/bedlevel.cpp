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

#if HAS_LEVELING

#include "bedlevel.h"
#include "../../module/planner.h"

#if ENABLED(PROBE_MANUALLY)
  #include "../../module/motion.h"
#endif

#if ENABLED(PROBE_MANUALLY)
  bool g29_in_progress = false;
#endif

#if ENABLED(LCD_BED_LEVELING)
  #include "../../lcd/ultralcd.h"
#endif

#define DEBUG_OUT ENABLED(DEBUG_LEVELING_FEATURE)
#include "../../core/debug_out.h"

#if ENABLED(EXTENSIBLE_UI)
  #include "../../lcd/extensible_ui/ui_api.h"
#endif

bool leveling_is_valid() {
  return
    #if ENABLED(AUTO_BED_LEVELING_UBL)
      ubl.mesh_is_valid()
    #else // 3POINT, LINEAR
      true
    #endif
  ;
}

/**
 * Turn bed leveling on or off, fixing the current
 * position as-needed.
 *
 * Disable: Current position = physical position
 *  Enable: Current position = "unleveled" physical position
 */
void set_bed_leveling_enabled(const bool enable/*=true*/) {

  constexpr bool can_change = true;

  if (can_change && enable != planner.leveling_active) {

    planner.synchronize();

    if (planner.leveling_active) {      // leveling from on to off
      // change unleveled current_position to physical current_position without moving steppers.
      planner.apply_leveling(current_position);
      planner.leveling_active = false;  // disable only AFTER calling apply_leveling
    }
    else {                              // leveling from off to on
      planner.leveling_active = true;   // enable BEFORE calling unapply_leveling, otherwise ignored
      // change physical current_position to unleveled current_position without moving steppers.
      planner.unapply_leveling(current_position);
    }

    sync_plan_position();
  }
}

TemporaryBedLevelingState::TemporaryBedLevelingState(const bool enable) : saved(planner.leveling_active) {
  set_bed_leveling_enabled(enable);
}

#if ENABLED(ENABLE_LEVELING_FADE_HEIGHT)

  void set_z_fade_height(const float zfh, const bool do_report/*=true*/) {

    if (planner.z_fade_height == zfh) return;

    const bool leveling_was_active = planner.leveling_active;
    set_bed_leveling_enabled(false);

    planner.set_z_fade_height(zfh);

    if (leveling_was_active) {
      const xyz_pos_t oldpos = current_position;
      set_bed_leveling_enabled(true);
      if (do_report && oldpos != current_position)
        report_current_position();
    }
  }

#endif // ENABLE_LEVELING_FADE_HEIGHT

/**
 * Reset calibration results to zero.
 */
void reset_bed_level() {
  if (DEBUGGING(LEVELING)) DEBUG_ECHOLNPGM("reset_bed_level");
  #if ENABLED(AUTO_BED_LEVELING_UBL)
    ubl.reset();
  #else
    set_bed_leveling_enabled(false);
  #endif
}

#if ENABLED(PROBE_MANUALLY)

  void _manual_goto_xy(const xy_pos_t &pos) {

    #ifdef MANUAL_PROBE_START_Z
      constexpr float startz = _MAX(0, MANUAL_PROBE_START_Z);
      #if MANUAL_PROBE_HEIGHT > 0
        do_blocking_move_to_xy_z(pos, MANUAL_PROBE_HEIGHT);
        do_blocking_move_to_z(startz);
      #else
        do_blocking_move_to_xy_z(pos, startz);
      #endif
    #elif MANUAL_PROBE_HEIGHT > 0
      const float prev_z = current_position.z;
      do_blocking_move_to_xy_z(pos, MANUAL_PROBE_HEIGHT);
      do_blocking_move_to_z(prev_z);
    #else
      do_blocking_move_to_xy(pos);
    #endif

    current_position = pos;

    #if ENABLED(LCD_BED_LEVELING)
      ui.wait_for_bl_move = false;
    #endif
  }

#endif

#endif // HAS_LEVELING
