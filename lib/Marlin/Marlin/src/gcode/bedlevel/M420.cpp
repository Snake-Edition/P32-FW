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

#include "../gcode.h"
#include "../../feature/bedlevel/bedlevel.h"
#include "../../module/planner.h"
#include "../../module/probe.h"

#if ENABLED(EEPROM_SETTINGS)
  #include "../../module/configuration_store.h"
#endif

#if ENABLED(EXTENSIBLE_UI)
  #include "../../lcd/extensible_ui/ui_api.h"
#endif

//#define M420_C_USE_MEAN

/** \addtogroup G-Codes
 * @{
 */

/**
 *### M420: Enable/Disable Bed Leveling and/or set the Z fade height  <a href="https://reprap.org/wiki/G-code#M420:_Leveling_On.2FOff.2FFade_.28Marlin.29">M420: Leveling On/Off/Fade</a>
 *
 *#### Usage
 *
 *    M420 [ S | Z | V | L | T | C ]
 *
 *#### Parameters
 *
 *
 * - `S`- Set leveling
 *    - `0` - off
 *    - `1` - on
 *    - `2` - Create a simple random mesh and enable
 * - `Z`- Sets the Z fade height (0 or none to disable)
 * - `V`- Verbose - Print the leveling grid
 * - `L`- Load UBL mesh from index (0 is default)
 * - `T`- Format to print the mesh data
 *    - `0` - Human-readable
 *    - `1` - CSV
 *    - `2` - LCD
 *    - `4` - Compact
 * - `C`- Center mesh on the mean of the lowest and highest
 *
 * Without parameters prints the current Bed Leveling and Z fade height
 */
void GcodeSuite::M420() {
  const bool seen_S = parser.seen('S'),
             to_enable = seen_S ? parser.value_bool() : planner.leveling_active;

  #if ENABLED(MARLIN_DEV_MODE)
    if (parser.intval('S') == 2) {
      #if ENABLED(AUTO_BED_LEVELING_BILINEAR)
        const float x_min = probe_min_x(), x_max = probe_max_x(),
                    y_min = probe_min_y(), y_max = probe_max_y();
        bilinear_start.set(x_min, y_min);
        bilinear_grid_spacing.set((x_max - x_min) / (GRID_MAX_POINTS_X - 1),
                                  (y_max - y_min) / (GRID_MAX_POINTS_Y - 1));
      #endif
      for (uint8_t x = 0; x < GRID_MAX_POINTS_X; x++)
        for (uint8_t y = 0; y < GRID_MAX_POINTS_Y; y++) {
          Z_VALUES(x, y) = 0.001 * random(-200, 200);
          #if ENABLED(EXTENSIBLE_UI)
            ExtUI::onMeshUpdate(x, y, Z_VALUES(x, y));
          #endif
        }
      SERIAL_ECHOPGM("Simulated " STRINGIFY(GRID_MAX_POINTS_X) "x" STRINGIFY(GRID_MAX_POINTS_Y) " mesh ");
      SERIAL_ECHOPAIR(" (", x_min);
      SERIAL_CHAR(','); SERIAL_ECHO(y_min);
      SERIAL_ECHOPAIR(")-(", x_max);
      SERIAL_CHAR(','); SERIAL_ECHO(y_max);
      SERIAL_ECHOLNPGM(")");
    }
  #endif

  // If disabling leveling do it right away
  // (Don't disable for just M420 or M420 V)
  if (seen_S && !to_enable) set_bed_leveling_enabled(false);

  xyz_pos_t oldpos = current_position;

  #if ENABLED(AUTO_BED_LEVELING_UBL)

    // L to load a mesh from the EEPROM
    if (parser.seen('L')) {

      set_bed_leveling_enabled(false);

      #if ENABLED(EEPROM_SETTINGS)
        const int8_t storage_slot = parser.has_value() ? parser.value_int() : ubl.storage_slot;
        const int16_t a = settings.calc_num_meshes();

        if (!a) {
          SERIAL_ECHOLNPGM("?EEPROM storage not available.");
          return;
        }

        if (!WITHIN(storage_slot, 0, a - 1)) {
          SERIAL_ECHOLNPGM("?Invalid storage slot.");
          SERIAL_ECHOLNPAIR("?Use 0 to ", a - 1);
          return;
        }

        settings.load_mesh(storage_slot);
        ubl.storage_slot = storage_slot;

      #else

        SERIAL_ECHOLNPGM("?EEPROM storage not available.");
        return;

      #endif
    }

    // L or V display the map info
    if (parser.seen("LV")) {
      ubl.display_map(parser.byteval('T'));
      SERIAL_ECHOPGM("Mesh is ");
      if (!ubl.mesh_is_valid()) SERIAL_ECHOPGM("in");
      SERIAL_ECHOLNPAIR("valid\nStorage slot: ", ubl.storage_slot);
    }

  #endif // AUTO_BED_LEVELING_UBL

  const bool seenV = parser.seen('V');

  #if HAS_MESH

    if (leveling_is_valid()) {

      // Subtract the given value or the mean from all mesh values
      if (parser.seen('C')) {
        const float cval = parser.value_float();
        #if ENABLED(AUTO_BED_LEVELING_UBL)

          set_bed_leveling_enabled(false);
          ubl.adjust_mesh_to_mean(true, cval);

        #else

          #if ENABLED(M420_C_USE_MEAN)

            // Get the sum and average of all mesh values
            float mesh_sum = 0;
            for (uint8_t x = GRID_MAX_POINTS_X; x--;)
              for (uint8_t y = GRID_MAX_POINTS_Y; y--;)
                mesh_sum += Z_VALUES(x, y);
            const float zmean = mesh_sum / float(GRID_MAX_POINTS);

          #else

            // Find the low and high mesh values
            float lo_val = 100, hi_val = -100;
            for (uint8_t x = GRID_MAX_POINTS_X; x--;)
              for (uint8_t y = GRID_MAX_POINTS_Y; y--;) {
                const float z = Z_VALUES(x, y);
                NOMORE(lo_val, z);
                NOLESS(hi_val, z);
              }
            // Take the mean of the lowest and highest
            const float zmean = (lo_val + hi_val) / 2.0 + cval;

          #endif

          // If not very close to 0, adjust the mesh
          if (!NEAR_ZERO(zmean)) {
            set_bed_leveling_enabled(false);
            // Subtract the mean from all values
            for (uint8_t x = GRID_MAX_POINTS_X; x--;)
              for (uint8_t y = GRID_MAX_POINTS_Y; y--;) {
                Z_VALUES(x, y) -= zmean;
                #if ENABLED(EXTENSIBLE_UI)
                  ExtUI::onMeshUpdate(x, y, Z_VALUES(x, y));
                #endif
              }
            #if ENABLED(ABL_BILINEAR_SUBDIVISION)
              bed_level_virt_interpolate();
            #endif
          }

        #endif
      }

    }
    else if (to_enable || seenV) {
      SERIAL_ECHO_MSG("Invalid mesh.");
      goto EXIT_M420;
    }

  #endif // HAS_MESH

  // V to print the matrix or mesh
  if (seenV) {
    #if ABL_PLANAR
      planner.bed_level_matrix.debug(PSTR("Bed Level Correction Matrix:"));
    #else
      if (leveling_is_valid()) {
        #if ENABLED(AUTO_BED_LEVELING_BILINEAR)
          print_bilinear_leveling_grid();
          #if ENABLED(ABL_BILINEAR_SUBDIVISION)
            print_bilinear_leveling_grid_virt();
          #endif
        #elif ENABLED(MESH_BED_LEVELING)
          SERIAL_ECHOLNPGM("Mesh Bed Level data:");
          mbl.report_mesh();
        #endif
      }
    #endif
  }

  #if ENABLED(ENABLE_LEVELING_FADE_HEIGHT)
    if (parser.seen('Z')) set_z_fade_height(parser.value_linear_units(), false);
  #endif

  // Enable leveling if specified, or if previously active
  set_bed_leveling_enabled(to_enable);

  #if HAS_MESH
    EXIT_M420:
  #endif

  // Error if leveling failed to enable or reenable
  if (to_enable && !planner.leveling_active)
    SERIAL_ERROR_MSG(MSG_ERR_M420_FAILED);

  SERIAL_ECHO_START();
  SERIAL_ECHOPGM("Bed Leveling ");
  serialprintln_onoff(planner.leveling_active);

  #if ENABLED(ENABLE_LEVELING_FADE_HEIGHT)
    SERIAL_ECHO_START();
    SERIAL_ECHOPGM("Fade Height ");
    if (planner.z_fade_height > 0.0)
      SERIAL_ECHOLN(planner.z_fade_height);
    else
      SERIAL_ECHOLNPGM(MSG_OFF);
  #endif

  // Report change in position
  if (oldpos != current_position)
    report_current_position();
}

/** @}*/

#endif // HAS_LEVELING
