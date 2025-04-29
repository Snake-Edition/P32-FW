/**
 * Marlin 3D Printer Firmware
 * Copyright (C) 2019 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (C) 2011 Camiel Gubbels / Erik van der Zalm
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

#include "hwio_pindef.h"
#include <device/board.h>

#define DEFAULT_MACHINE_NAME "Prusa-Dwarf"
#define BOARD_NAME "Dwarf"

//#define I2C_EEPROM

//#define E2END 0x03ff // EEPROM end address (1kB)

#if HOTENDS > 1 || E_STEPPERS > 1
  #error "Buddy supports up to 1 hotends / E-steppers."
#endif
#if !BOARD_IS_DWARF()
  #error Wrong board
#endif

//
// Limit Switches
//
#define Z_MIN_PIN              MARLIN_PIN(Z_DIAG)
#define Z_MAX_PIN              MARLIN_PIN(Z_DIAG)

//
// Z Probe (when not Z_MIN_PIN)
//

//
// Steppers
//

#define E0_STEP_PIN            MARLIN_PIN(E0_STEP)
#define E0_DIR_PIN             MARLIN_PIN(E0_DIR)
#define E0_ENABLE_PIN          MARLIN_PIN(E0_ENA)

#if !HAS_DRIVER(TMC2130)
  #error Unsupported driver
#endif
#define E0_CS_PIN MARLIN_PIN(CS_E)


//
// Temperature Sensors
//

#define TEMP_0_PIN             MARLIN_PIN(TEMP_0)     // Analog Input

#define TEMP_BOARD_PIN         MARLIN_PIN(TEMP_BOARD) // Analog Input
#define TEMP_HEATBREAK_PIN     MARLIN_PIN(TEMP_HEATBREAK) // Analog Input

//
// Heaters / Fans
//

#define HEATER_0_PIN             MARLIN_PIN(HEAT0)

#define FAN_PIN                MARLIN_PIN(FAN)

#define HEATER_HEATBREAK_PIN   MARLIN_PIN(FAN1)
#define FAN1_PIN               MARLIN_PIN(FAN1)

#if defined(MARLIN_PORT_HEATER_ENABLE) && defined(MARLIN_PIN_NR_HEATER_ENABLE)
  #define PS_ON_PIN              MARLIN_PIN(HEATER_ENABLE)
#endif
