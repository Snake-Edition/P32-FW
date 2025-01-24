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

class MarlinSettings {
  public:
    static void reset_motion(const bool no_limits=false);
    static void reset();
    static bool save();    // Return 'true' if data was saved

    FORCE_INLINE static bool init_eeprom() {
      reset();
      return true;
    }

    #if ENABLED(SD_FIRMWARE_UPDATE)
      static bool sd_update_status();                       // True if the SD-Firmware-Update EEPROM flag is set
      static bool set_sd_update_status(const bool enable);  // Return 'true' after EEPROM is set (-> always true)
    #endif

    FORCE_INLINE
    static bool load() { reset(); report(); return true; }
    FORCE_INLINE
    static void first_load() { (void)load(); }

    #if DISABLED(DISABLE_M503)
      static void report(const bool forReplay=false);
    #else
      FORCE_INLINE
      static void report(const bool=false) {}
    #endif

  private:
    static void postprocess();
};

extern MarlinSettings settings;
