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

/**
 * Conditionals_post.h
 * Defines that depend on configuration but are not editable.
 */

#define AVR_ATmega2560_FAMILY_PLUS_70 ( \
     MB(BQ_ZUM_MEGA_3D)                 \
  || MB(MIGHTYBOARD_REVE)               \
  || MB(MINIRAMBO)                      \
  || MB(SCOOVO_X9H)                     \
)

#ifdef TEENSYDUINO
  #undef max
  #define max(a,b) ((a)>(b)?(a):(b))
  #undef min
  #define min(a,b) ((a)<(b)?(a):(b))

  #undef NOT_A_PIN    // Override Teensyduino legacy CapSense define work-around
  #define NOT_A_PIN 0 // For PINS_DEBUGGING
#endif

#if (ENABLED(CLASSIC_JERK) || IS_KINEMATIC)
  #define HAS_CLASSIC_JERK 1
#endif
#define HAS_CLASSIC_E_JERK HAS_CLASSIC_JERK

/**
 * Axis lengths and center
 */
#define X_MAX_LENGTH (X_MAX_POS - (X_MIN_POS))
#define Y_MAX_LENGTH (Y_MAX_POS - (Y_MIN_POS))
#define Z_MAX_LENGTH (Z_MAX_POS - (Z_MIN_POS))

// Defined only if the sanity-check is bypassed
// #ifndef X_BED_SIZE
//   #define X_BED_SIZE X_MAX_LENGTH
// #endif
// #ifndef Y_BED_SIZE
//   #define Y_BED_SIZE Y_MAX_LENGTH
// #endif

// Require 0,0 bed center for Delta and SCARA
#if IS_KINEMATIC
  #define BED_CENTER_AT_0_0
#endif

// Define center values for future use
#define _X_HALF_BED ((X_BED_SIZE) / 2)
#define _Y_HALF_BED ((Y_BED_SIZE) / 2)
#if ENABLED(BED_CENTER_AT_0_0)
  #define X_CENTER 0
  #define Y_CENTER 0
#else
  #define X_CENTER _X_HALF_BED
  #define Y_CENTER _Y_HALF_BED
#endif

// Get the linear boundaries of the bed
#define X_MIN_BED (X_CENTER - _X_HALF_BED)
#define X_MAX_BED (X_MIN_BED + X_BED_SIZE)
#define Y_MIN_BED (Y_CENTER - _Y_HALF_BED)
#define Y_MAX_BED (Y_MIN_BED + Y_BED_SIZE)

/**
 * Dual X Carriage
 */
#if ENABLED(DUAL_X_CARRIAGE)
  #ifndef X1_MIN_POS
    #define X1_MIN_POS X_MIN_POS
  #endif
  #ifndef X1_MAX_POS
    #define X1_MAX_POS X_BED_SIZE
  #endif
#endif

/**
 * CoreXY, CoreXZ, and CoreYZ - and their reverse
 */
#if EITHER(COREXY, COREYX)
  #define CORE_IS_XY 1
#endif
#define CORE_IS_XZ EITHER(COREXZ, COREZX)
#define CORE_IS_YZ EITHER(COREYZ, COREZY)
#if (CORE_IS_XY || CORE_IS_XZ || CORE_IS_YZ)
  #define IS_CORE 1
#endif
#if IS_CORE
  #if CORE_IS_XY
    #define CORE_AXIS_1 A_AXIS
    #define CORE_AXIS_2 B_AXIS
    #define NORMAL_AXIS Z_AXIS
  #elif CORE_IS_XZ
    #define CORE_AXIS_1 A_AXIS
    #define NORMAL_AXIS Y_AXIS
    #define CORE_AXIS_2 C_AXIS
  #elif CORE_IS_YZ
    #define NORMAL_AXIS X_AXIS
    #define CORE_AXIS_1 B_AXIS
    #define CORE_AXIS_2 C_AXIS
  #endif
  #if ANY(COREYX, COREZX, COREZY)
    #define CORESIGN(n) (-(n))
  #else
    #define CORESIGN(n) (n)
  #endif
#endif

/**
 * No adjustable bed on non-cartesians
 */
#if IS_KINEMATIC
  #undef LEVEL_BED_CORNERS
#endif

/**
 * SCARA cannot use SLOWDOWN and requires QUICKHOME
 */
#if IS_SCARA
  #undef SLOWDOWN
  #define QUICK_HOME
#endif

/**
 * Set the home position based on settings or manual overrides
 */
#ifdef MANUAL_X_HOME_POS
  #define X_HOME_POS MANUAL_X_HOME_POS
#elif ENABLED(BED_CENTER_AT_0_0)
  #if ENABLED(DELTA)
    #define X_HOME_POS 0
  #else
    #define X_HOME_POS (X_HOME_DIR < 0 ? X_MIN_POS : X_MAX_POS)
  #endif
#else
  #if ENABLED(DELTA)
    #define X_HOME_POS (X_MIN_POS + (X_BED_SIZE) * 0.5)
  #else
    #define X_HOME_POS (X_HOME_DIR < 0 ? X_MIN_POS : X_MAX_POS)
  #endif
#endif

#ifdef MANUAL_Y_HOME_POS
  #define Y_HOME_POS MANUAL_Y_HOME_POS
#elif ENABLED(BED_CENTER_AT_0_0)
  #if ENABLED(DELTA)
    #define Y_HOME_POS 0
  #else
    #define Y_HOME_POS (Y_HOME_DIR < 0 ? Y_MIN_POS : Y_MAX_POS)
  #endif
#else
  #if ENABLED(DELTA)
    #define Y_HOME_POS (Y_MIN_POS + (Y_BED_SIZE) * 0.5)
  #else
    #define Y_HOME_POS (Y_HOME_DIR < 0 ? Y_MIN_POS : Y_MAX_POS)
  #endif
#endif

#ifdef MANUAL_Z_HOME_POS
  #define Z_HOME_POS MANUAL_Z_HOME_POS
#else
  #define Z_HOME_POS (Z_HOME_DIR < 0 ? Z_MIN_POS : Z_MAX_POS)
#endif

/**
 * If DELTA_HEIGHT isn't defined use the old setting
 */
#if ENABLED(DELTA) && !defined(DELTA_HEIGHT)
  #define DELTA_HEIGHT Z_HOME_POS
#endif

/**
 * Z Sled Probe requires Z_SAFE_HOMING
 */
#if ENABLED(Z_PROBE_SLED)
  #define Z_SAFE_HOMING
#endif

/**
 * DELTA should ignore Z_SAFE_HOMING and SLOWDOWN
 */
#if ENABLED(DELTA)
  #undef Z_SAFE_HOMING
  #undef SLOWDOWN
#endif

#ifndef MESH_INSET
  #define MESH_INSET 0
#endif

/**
 * Safe Homing Options
 */
#if ENABLED(Z_SAFE_HOMING)
  #if ENABLED(AUTO_BED_LEVELING_UBL)
    // Home close to center so grid points have z heights very close to 0
    #define _SAFE_POINT(A) (((GRID_MAX_POINTS_##A) / 2) * (A##_BED_SIZE - 2 * (MESH_INSET)) / (GRID_MAX_POINTS_##A - 1) + MESH_INSET)
  #else
    #define _SAFE_POINT(A) A##_CENTER
  #endif
  #ifndef Z_SAFE_HOMING_X_POINT
    #define Z_SAFE_HOMING_X_POINT _SAFE_POINT(X)
  #endif
  #ifndef Z_SAFE_HOMING_Y_POINT
    #define Z_SAFE_HOMING_Y_POINT _SAFE_POINT(Y)
  #endif
#endif

/**
 * Host keep alive
 */
#ifndef DEFAULT_KEEPALIVE_INTERVAL
  #define DEFAULT_KEEPALIVE_INTERVAL 2
#endif

/**
 * Provide a MAX_AUTORETRACT for older configs
 */
#if ENABLED(FWRETRACT) && !defined(MAX_AUTORETRACT)
  #define MAX_AUTORETRACT 99
#endif

/**
 * LCD Contrast for Graphical Displays
 */
#if ENABLED(CARTESIO_UI)
  #define _LCD_CONTRAST_MIN   60
  #define _LCD_CONTRAST_INIT  90
  #define _LCD_CONTRAST_MAX  140
#elif ENABLED(miniVIKI)
  #define _LCD_CONTRAST_MIN   75
  #define _LCD_CONTRAST_INIT  95
  #define _LCD_CONTRAST_MAX  115
#elif ENABLED(VIKI2)
  #define _LCD_CONTRAST_INIT 140
#elif ENABLED(ELB_FULL_GRAPHIC_CONTROLLER)
  #define _LCD_CONTRAST_MIN   90
  #define _LCD_CONTRAST_INIT 110
  #define _LCD_CONTRAST_MAX  130
#elif ENABLED(AZSMZ_12864)
  #define _LCD_CONTRAST_MIN  120
  #define _LCD_CONTRAST_INIT 190
#elif ENABLED(MKS_MINI_12864)
  #define _LCD_CONTRAST_MIN  120
  #define _LCD_CONTRAST_INIT 195
#elif ANY(FYSETC_MINI_12864_X_X, FYSETC_MINI_12864_1_2, FYSETC_MINI_12864_2_0, FYSETC_MINI_12864_2_1)
  #define _LCD_CONTRAST_INIT 220
#elif ENABLED(ULTI_CONTROLLER)
  #define _LCD_CONTRAST_INIT 127
  #define _LCD_CONTRAST_MAX  254
#elif EITHER(MAKRPANEL, MINIPANEL)
  #define _LCD_CONTRAST_INIT  17
#endif

#define HAS_LCD_CONTRAST defined(_LCD_CONTRAST_INIT)
#if HAS_LCD_CONTRAST
  #ifndef LCD_CONTRAST_MIN
    #ifdef _LCD_CONTRAST_MIN
      #define LCD_CONTRAST_MIN _LCD_CONTRAST_MIN
    #else
      #define LCD_CONTRAST_MIN 0
    #endif
  #endif
  #ifndef LCD_CONTRAST_INIT
    #define LCD_CONTRAST_INIT _LCD_CONTRAST_INIT
  #endif
  #ifndef LCD_CONTRAST_MAX
    #ifdef _LCD_CONTRAST_MAX
      #define LCD_CONTRAST_MAX _LCD_CONTRAST_MAX
    #elif _LCD_CONTRAST_INIT > 63
      #define LCD_CONTRAST_MAX 255
    #else
      #define LCD_CONTRAST_MAX 63   // ST7567 6-bits contrast
    #endif
  #endif
  #ifndef DEFAULT_LCD_CONTRAST
    #define DEFAULT_LCD_CONTRAST LCD_CONTRAST_INIT
  #endif
#endif

/**
 * Override here because this is set in Configuration_adv.h
 */
#if HAS_LCD_MENU && DISABLED(ELB_FULL_GRAPHIC_CONTROLLER)
  #undef SD_DETECT_INVERTED
#endif

/**
 * Set defaults for missing (newer) options
 */
#ifndef DISABLE_INACTIVE_X
  #define DISABLE_INACTIVE_X DISABLE_X
#endif
#ifndef DISABLE_INACTIVE_Y
  #define DISABLE_INACTIVE_Y DISABLE_Y
#endif
#ifndef DISABLE_INACTIVE_Z
  #define DISABLE_INACTIVE_Z DISABLE_Z
#endif
#ifndef DISABLE_INACTIVE_E
  #define DISABLE_INACTIVE_E DISABLE_E
#endif

/**
 * Power Supply Control
 */
#ifndef PSU_NAME
  #if ENABLED(PSU_CONTROL)
    #if PSU_ACTIVE_HIGH
      #define PSU_NAME "XBox"     // X-Box 360 (203W)
    #else
      #define PSU_NAME "ATX"      // ATX style
    #endif
  #else
    #define PSU_NAME "Generic"    // No control
  #endif
#endif

#define HAS_POWER_SWITCH (ENABLED(PSU_CONTROL) && PIN_EXISTS(PS_ON))

/**
 * Temp Sensor defines
 */

#define ANY_TEMP_SENSOR_IS(n) (TEMP_SENSOR_0 == (n) || TEMP_SENSOR_1 == (n) || TEMP_SENSOR_2 == (n) || TEMP_SENSOR_3 == (n) || TEMP_SENSOR_4 == (n) || TEMP_SENSOR_5 == (n) || TEMP_SENSOR_BED == (n) || TEMP_SENSOR_CHAMBER == (n))

#define HAS_USER_THERMISTORS ANY_TEMP_SENSOR_IS(1000)

#if TEMP_SENSOR_0 == -4
  #define HEATER_0_USES_AD8495
#elif TEMP_SENSOR_0 == -3
  #define HEATER_0_USES_MAX6675
  #define MAX6675_IS_MAX31855
  #define HEATER_0_MAX6675_TMIN -270
  #define HEATER_0_MAX6675_TMAX 1800
#elif TEMP_SENSOR_0 == -2
  #define HEATER_0_USES_MAX6675
  #define HEATER_0_MAX6675_TMIN 0
  #define HEATER_0_MAX6675_TMAX 1024
#elif TEMP_SENSOR_0 == -1
  #define HEATER_0_USES_AD595
#elif TEMP_SENSOR_0 > 0
  #define THERMISTOR_HEATER_0 TEMP_SENSOR_0
  #define HEATER_0_USES_THERMISTOR
  #if TEMP_SENSOR_0 == 1000
    #define HEATER_0_USER_THERMISTOR
  #endif
#else
  #undef HEATER_0_MINTEMP
  #undef HEATER_0_MAXTEMP
#endif

#if TEMP_SENSOR_1 == -4
  #define HEATER_1_USES_AD8495
#elif TEMP_SENSOR_1 == -3
  #if TEMP_SENSOR_0 == -2
    #error "If MAX31855 Thermocouple (-3) is used for TEMP_SENSOR_1 then TEMP_SENSOR_0 must match."
  #endif
  #define HEATER_1_USES_MAX6675
  #define HEATER_1_MAX6675_TMIN -270
  #define HEATER_1_MAX6675_TMAX 1800
#elif TEMP_SENSOR_1 == -2
  #if TEMP_SENSOR_0 == -3
    #error "If MAX31855 Thermocouple (-3) is used for TEMP_SENSOR_0 then TEMP_SENSOR_1 must match."
  #endif
  #define HEATER_1_USES_MAX6675
  #define HEATER_1_MAX6675_TMIN 0
  #define HEATER_1_MAX6675_TMAX 1024
#elif TEMP_SENSOR_1 == -1
  #define HEATER_1_USES_AD595
#elif TEMP_SENSOR_1 > 0
  #define THERMISTOR_HEATER_1 TEMP_SENSOR_1
  #define HEATER_1_USES_THERMISTOR
  #if TEMP_SENSOR_1 == 1000
    #define HEATER_1_USER_THERMISTOR
  #endif
#else
  #undef HEATER_1_MINTEMP
  #undef HEATER_1_MAXTEMP
#endif

#if TEMP_SENSOR_2 == -4
  #define HEATER_2_USES_AD8495
#elif TEMP_SENSOR_2 == -3
  #error "MAX31855 Thermocouples (-3) not supported for TEMP_SENSOR_2."
#elif TEMP_SENSOR_2 == -2
  #error "MAX6675 Thermocouples (-2) not supported for TEMP_SENSOR_2."
#elif TEMP_SENSOR_2 == -1
  #define HEATER_2_USES_AD595
#elif TEMP_SENSOR_2 > 0
  #define THERMISTOR_HEATER_2 TEMP_SENSOR_2
  #define HEATER_2_USES_THERMISTOR
  #if TEMP_SENSOR_2 == 1000
    #define HEATER_2_USER_THERMISTOR
  #endif
#else
  #undef HEATER_2_MINTEMP
  #undef HEATER_2_MAXTEMP
#endif

#if TEMP_SENSOR_3 == -4
  #define HEATER_3_USES_AD8495
#elif TEMP_SENSOR_3 == -3
  #error "MAX31855 Thermocouples (-3) not supported for TEMP_SENSOR_3."
#elif TEMP_SENSOR_3 == -2
  #error "MAX6675 Thermocouples (-2) not supported for TEMP_SENSOR_3."
#elif TEMP_SENSOR_3 == -1
  #define HEATER_3_USES_AD595
#elif TEMP_SENSOR_3 > 0
  #define THERMISTOR_HEATER_3 TEMP_SENSOR_3
  #define HEATER_3_USES_THERMISTOR
  #if TEMP_SENSOR_3 == 1000
    #define HEATER_3_USER_THERMISTOR
  #endif
#else
  #undef HEATER_3_MINTEMP
  #undef HEATER_3_MAXTEMP
#endif

#if TEMP_SENSOR_4 == -4
  #define HEATER_4_USES_AD8495
#elif TEMP_SENSOR_4 == -3
  #error "MAX31855 Thermocouples (-3) not supported for TEMP_SENSOR_4."
#elif TEMP_SENSOR_4 == -2
  #error "MAX6675 Thermocouples (-2) not supported for TEMP_SENSOR_4."
#elif TEMP_SENSOR_4 == -1
  #define HEATER_4_USES_AD595
#elif TEMP_SENSOR_4 > 0
  #define THERMISTOR_HEATER_4 TEMP_SENSOR_4
  #define HEATER_4_USES_THERMISTOR
  #if TEMP_SENSOR_4 == 1000
    #define HEATER_4_USER_THERMISTOR
  #endif
#else
  #undef HEATER_4_MINTEMP
  #undef HEATER_4_MAXTEMP
#endif

#if TEMP_SENSOR_5 == -4
  #define HEATER_5_USES_AD8495
#elif TEMP_SENSOR_5 == -3
  #error "MAX31855 Thermocouples (-3) not supported for TEMP_SENSOR_5."
#elif TEMP_SENSOR_5 == -2
  #error "MAX6675 Thermocouples (-2) not supported for TEMP_SENSOR_5."
#elif TEMP_SENSOR_5 == -1
  #define HEATER_5_USES_AD595
#elif TEMP_SENSOR_5 > 0
  #define THERMISTOR_HEATER_5 TEMP_SENSOR_5
  #define HEATER_5_USES_THERMISTOR
  #if TEMP_SENSOR_5 == 1000
    #define HEATER_5_USER_THERMISTOR
  #endif
#else
  #undef HEATER_5_MINTEMP
  #undef HEATER_5_MAXTEMP
#endif

#if TEMP_SENSOR_BED == -4
  #define HEATER_BED_USES_AD8495
#elif TEMP_SENSOR_BED == -3
  #error "MAX31855 Thermocouples (-3) not supported for TEMP_SENSOR_BED."
#elif TEMP_SENSOR_BED == -2
  #error "MAX6675 Thermocouples (-2) not supported for TEMP_SENSOR_BED."
#elif TEMP_SENSOR_BED == -1
  #define HEATER_BED_USES_AD595
#elif TEMP_SENSOR_BED > 0
  #define THERMISTORBED TEMP_SENSOR_BED
  #define HEATER_BED_USES_THERMISTOR
  #if TEMP_SENSOR_BED == 1000
    #define HEATER_BED_USER_THERMISTOR
  #endif
#else
  #undef BED_MINTEMP
  #undef BED_MAXTEMP
#endif

#if TEMP_SENSOR_CHAMBER == -4
  #define HEATER_CHAMBER_USES_AD8495
#elif TEMP_SENSOR_CHAMBER == -3
  #error "MAX31855 Thermocouples (-3) not supported for TEMP_SENSOR_CHAMBER."
#elif TEMP_SENSOR_CHAMBER == -2
  #error "MAX6675 Thermocouples (-2) not supported for TEMP_SENSOR_CHAMBER."
#elif TEMP_SENSOR_CHAMBER == -1
  #define HEATER_CHAMBER_USES_AD595
#elif TEMP_SENSOR_CHAMBER > 0
  #define THERMISTORCHAMBER TEMP_SENSOR_CHAMBER
  #define HEATER_CHAMBER_USES_THERMISTOR
  #if TEMP_SENSOR_CHAMBER == 1000
    #define HEATER_CHAMBER_USER_THERMISTOR
  #endif
#else
  #undef CHAMBER_MINTEMP
  #undef CHAMBER_MAXTEMP
#endif


#if TEMP_SENSOR_HEATBREAK == -4
  #error "AD8495 Thermocouples (-4) not supported for TEMP_SENSOR_HEATBREAK."
#elif TEMP_SENSOR_HEATBREAK == -3
  #error "MAX31855 Thermocouples (-3) not supported for TEMP_SENSOR_HEATBREAK."
#elif TEMP_SENSOR_HEATBREAK == -2
  #error "MAX6675 Thermocouples (-2) not supported for TEMP_SENSOR_HEATBREAK."
#elif TEMP_SENSOR_HEATBREAK == -1
  #error "AD595 Thermocouples (-1) not supported for TEMP_SENSOR_HEATBREAK."
#elif TEMP_SENSOR_HEATBREAK > 0
  #define THERMISTORHEATBREAK TEMP_SENSOR_HEATBREAK
  #define HEATBREAK_USES_THERMISTOR
  #if TEMP_SENSOR_HEATBREAK == 1000
    #define HEATBREAK_USER_THERMISTOR
  #endif
#else
  #undef HEATBREAK_MINTEMP
  #undef HEATBREAK_MAXTEMP
#endif


#if TEMP_SENSOR_BOARD == -4
  #error "AD8495 Thermocouples (-4) not supported for TEMP_SENSOR_BOARD."
#elif TEMP_SENSOR_BOARD == -3
  #error "MAX31855 Thermocouples (-3) not supported for TEMP_SENSOR_BOARD."
#elif TEMP_SENSOR_BOARD == -2
  #error "MAX6675 Thermocouples (-2) not supported for TEMP_SENSOR_BOARD."
#elif TEMP_SENSOR_BOARD == -1
  #error "AD595 Thermocouples (-1) not supported for TEMP_SENSOR_BOARD."
#elif TEMP_SENSOR_BOARD > 0
  #define THERMISTORBOARD TEMP_SENSOR_BOARD
  #define BOARD_USES_THERMISTOR
  #if TEMP_SENSOR_BOARD == 1000
    #define BOARD_USER_THERMISTOR
  #endif
#else
  #undef BOARD_MINTEMP
  #undef BOARD_MINTEMP
#endif

#define HOTEND_USES_THERMISTOR ANY(HEATER_0_USES_THERMISTOR, HEATER_1_USES_THERMISTOR, HEATER_2_USES_THERMISTOR, HEATER_3_USES_THERMISTOR, HEATER_4_USES_THERMISTOR)

/**
 * Default hotend offsets, if not defined
 */
#if HAS_HOTEND_OFFSET
  #ifndef HOTEND_OFFSET_X
    #define HOTEND_OFFSET_X { 0 } // X offsets for each extruder
  #endif
  #ifndef HOTEND_OFFSET_Y
    #define HOTEND_OFFSET_Y { 0 } // Y offsets for each extruder
  #endif
  #ifndef HOTEND_OFFSET_Z
    #define HOTEND_OFFSET_Z { 0 } // Z offsets for each extruder
  #endif
#endif

/**
 * Driver Timings
 * NOTE: Driver timing order is longest-to-shortest duration.
 *       Preserve this ordering when adding new drivers.
 */

#define TRINAMICS (HAS_TRINAMIC || HAS_DRIVER(TMC2130_STANDALONE) || HAS_DRIVER(TMC2208_STANDALONE) || HAS_DRIVER(TMC2209_STANDALONE) || HAS_DRIVER(TMC26X_STANDALONE) || HAS_DRIVER(TMC2660_STANDALONE) || HAS_DRIVER(TMC5130_STANDALONE) || HAS_DRIVER(TMC5160_STANDALONE) || HAS_DRIVER(TMC2160_STANDALONE))

#ifndef MINIMUM_STEPPER_POST_DIR_DELAY
  #if HAS_DRIVER(TB6560)
    #define MINIMUM_STEPPER_POST_DIR_DELAY 15000
  #elif HAS_DRIVER(TB6600)
    #define MINIMUM_STEPPER_POST_DIR_DELAY 1500
  #elif HAS_DRIVER(DRV8825)
    #define MINIMUM_STEPPER_POST_DIR_DELAY 650
  #elif HAS_DRIVER(LV8729)
    #define MINIMUM_STEPPER_POST_DIR_DELAY 500
  #elif HAS_DRIVER(A5984)
    #define MINIMUM_STEPPER_POST_DIR_DELAY 400
  #elif HAS_DRIVER(A4988)
    #define MINIMUM_STEPPER_POST_DIR_DELAY 200
  #elif TRINAMICS
    #define MINIMUM_STEPPER_POST_DIR_DELAY 20
  #else
    #define MINIMUM_STEPPER_POST_DIR_DELAY 0   // Expect at least 10µS since one Stepper ISR must transpire
  #endif
#endif

#ifndef MINIMUM_STEPPER_PRE_DIR_DELAY
  #define MINIMUM_STEPPER_PRE_DIR_DELAY MINIMUM_STEPPER_POST_DIR_DELAY
#endif

#ifndef MINIMUM_STEPPER_PULSE
  #if HAS_DRIVER(TB6560)
    #define MINIMUM_STEPPER_PULSE 30
  #elif HAS_DRIVER(TB6600)
    #define MINIMUM_STEPPER_PULSE 3
  #elif HAS_DRIVER(DRV8825)
    #define MINIMUM_STEPPER_PULSE 2
  #elif HAS_DRIVER(A4988) || HAS_DRIVER(A5984)
    #define MINIMUM_STEPPER_PULSE 1
  #elif HAS_DRIVER(LV8729)
    #define MINIMUM_STEPPER_PULSE 0
  #elif TRINAMICS
    #define MINIMUM_STEPPER_PULSE 0
  #else
    #define MINIMUM_STEPPER_PULSE 2
  #endif
#endif

#ifndef MAXIMUM_STEPPER_RATE
  #if HAS_DRIVER(TB6560)
    #define MAXIMUM_STEPPER_RATE 15000
  #elif HAS_DRIVER(TB6600)
    #define MAXIMUM_STEPPER_RATE 150000
  #elif HAS_DRIVER(LV8729)
    #define MAXIMUM_STEPPER_RATE 200000
  #elif HAS_DRIVER(DRV8825)
    #define MAXIMUM_STEPPER_RATE 250000
  #elif TRINAMICS
    #define MAXIMUM_STEPPER_RATE 400000
  #elif HAS_DRIVER(A4988)
    #define MAXIMUM_STEPPER_RATE 500000
  #else
    #define MAXIMUM_STEPPER_RATE 250000
  #endif
#endif

/**
 * X_DUAL_ENDSTOPS endstop reassignment
 */
#if ENABLED(X_DUAL_ENDSTOPS)
  #if X_HOME_DIR > 0
    #if X2_USE_ENDSTOP == _XMIN_

      #define X2_MAX_ENDSTOP_INVERTING X_MIN_ENDSTOP_INVERTING
      #define X2_MAX_PIN X_MIN_PIN

    #elif X2_USE_ENDSTOP == _XMAX_
      #define X2_MAX_ENDSTOP_INVERTING X_MAX_ENDSTOP_INVERTING
      #define X2_MAX_PIN X_MAX_PIN
    #elif X2_USE_ENDSTOP == _YMIN_
      #define X2_MAX_ENDSTOP_INVERTING Y_MIN_ENDSTOP_INVERTING
      #define X2_MAX_PIN Y_MIN_PIN
    #elif X2_USE_ENDSTOP == _YMAX_
      #define X2_MAX_ENDSTOP_INVERTING Y_MAX_ENDSTOP_INVERTING
      #define X2_MAX_PIN Y_MAX_PIN
    #elif X2_USE_ENDSTOP == _ZMIN_
      #define X2_MAX_ENDSTOP_INVERTING Z_MIN_ENDSTOP_INVERTING
      #define X2_MAX_PIN Z_MIN_PIN
    #elif X2_USE_ENDSTOP == _ZMAX_
      #define X2_MAX_ENDSTOP_INVERTING Z_MAX_ENDSTOP_INVERTING
      #define X2_MAX_PIN Z_MAX_PIN
    #else
      #define X2_MAX_ENDSTOP_INVERTING false
    #endif
    #define X2_MIN_ENDSTOP_INVERTING false
  #else
    #if X2_USE_ENDSTOP == _XMIN_
      #define X2_MIN_ENDSTOP_INVERTING X_MIN_ENDSTOP_INVERTING
      #define X2_MIN_PIN X_MIN_PIN
    #elif X2_USE_ENDSTOP == _XMAX_
      #define X2_MIN_ENDSTOP_INVERTING X_MAX_ENDSTOP_INVERTING
      #define X2_MIN_PIN X_MAX_PIN
    #elif X2_USE_ENDSTOP == _YMIN_
      #define X2_MIN_ENDSTOP_INVERTING Y_MIN_ENDSTOP_INVERTING
      #define X2_MIN_PIN Y_MIN_PIN
    #elif X2_USE_ENDSTOP == _YMAX_
      #define X2_MIN_ENDSTOP_INVERTING Y_MAX_ENDSTOP_INVERTING
      #define X2_MIN_PIN Y_MAX_PIN
    #elif X2_USE_ENDSTOP == _ZMIN_
      #define X2_MIN_ENDSTOP_INVERTING Z_MIN_ENDSTOP_INVERTING
      #define X2_MIN_PIN Z_MIN_PIN
    #elif X2_USE_ENDSTOP == _ZMAX_
      #define X2_MIN_ENDSTOP_INVERTING Z_MAX_ENDSTOP_INVERTING
      #define X2_MIN_PIN Z_MAX_PIN
    #else
      #define X2_MIN_ENDSTOP_INVERTING false
    #endif
    #define X2_MAX_ENDSTOP_INVERTING false
  #endif
#endif

// Is an endstop plug used for the X2 endstop?
#define IS_X2_ENDSTOP(A,M) (ENABLED(X_DUAL_ENDSTOPS) && X2_USE_ENDSTOP == _##A##M##_)

/**
 * Y_DUAL_ENDSTOPS endstop reassignment
 */
#if ENABLED(Y_DUAL_ENDSTOPS)
  #if Y_HOME_DIR > 0
    #if Y2_USE_ENDSTOP == _XMIN_
      #define Y2_MAX_ENDSTOP_INVERTING X_MIN_ENDSTOP_INVERTING
      #define Y2_MAX_PIN X_MIN_PIN
    #elif Y2_USE_ENDSTOP == _XMAX_
      #define Y2_MAX_ENDSTOP_INVERTING X_MAX_ENDSTOP_INVERTING
      #define Y2_MAX_PIN X_MAX_PIN
    #elif Y2_USE_ENDSTOP == _YMIN_
      #define Y2_MAX_ENDSTOP_INVERTING Y_MIN_ENDSTOP_INVERTING
      #define Y2_MAX_PIN Y_MIN_PIN
    #elif Y2_USE_ENDSTOP == _YMAX_
      #define Y2_MAX_ENDSTOP_INVERTING Y_MAX_ENDSTOP_INVERTING
      #define Y2_MAX_PIN Y_MAX_PIN
    #elif Y2_USE_ENDSTOP == _ZMIN_
      #define Y2_MAX_ENDSTOP_INVERTING Z_MIN_ENDSTOP_INVERTING
      #define Y2_MAX_PIN Z_MIN_PIN
    #elif Y2_USE_ENDSTOP == _ZMAX_
      #define Y2_MAX_ENDSTOP_INVERTING Z_MAX_ENDSTOP_INVERTING
      #define Y2_MAX_PIN Z_MAX_PIN
    #else
      #define Y2_MAX_ENDSTOP_INVERTING false
    #endif
    #define Y2_MIN_ENDSTOP_INVERTING false
  #else
    #if Y2_USE_ENDSTOP == _XMIN_
      #define Y2_MIN_ENDSTOP_INVERTING X_MIN_ENDSTOP_INVERTING
      #define Y2_MIN_PIN X_MIN_PIN
    #elif Y2_USE_ENDSTOP == _XMAX_
      #define Y2_MIN_ENDSTOP_INVERTING X_MAX_ENDSTOP_INVERTING
      #define Y2_MIN_PIN X_MAX_PIN
    #elif Y2_USE_ENDSTOP == _YMIN_
      #define Y2_MIN_ENDSTOP_INVERTING Y_MIN_ENDSTOP_INVERTING
      #define Y2_MIN_PIN Y_MIN_PIN
    #elif Y2_USE_ENDSTOP == _YMAX_
      #define Y2_MIN_ENDSTOP_INVERTING Y_MAX_ENDSTOP_INVERTING
      #define Y2_MIN_PIN Y_MAX_PIN
    #elif Y2_USE_ENDSTOP == _ZMIN_
      #define Y2_MIN_ENDSTOP_INVERTING Z_MIN_ENDSTOP_INVERTING
      #define Y2_MIN_PIN Z_MIN_PIN
    #elif Y2_USE_ENDSTOP == _ZMAX_
      #define Y2_MIN_ENDSTOP_INVERTING Z_MAX_ENDSTOP_INVERTING
      #define Y2_MIN_PIN Z_MAX_PIN
    #else
      #define Y2_MIN_ENDSTOP_INVERTING false
    #endif
    #define Y2_MAX_ENDSTOP_INVERTING false
  #endif
#endif

// Is an endstop plug used for the Y2 endstop or the bed probe?
#define IS_Y2_ENDSTOP(A,M) (ENABLED(Y_DUAL_ENDSTOPS) && Y2_USE_ENDSTOP == _##A##M##_)

/**
 * Z_DUAL_ENDSTOPS endstop reassignment
 */
#if Z_MULTI_ENDSTOPS
  #if Z_HOME_DIR > 0
    #if Z2_USE_ENDSTOP == _XMIN_
      #define Z2_MAX_ENDSTOP_INVERTING X_MIN_ENDSTOP_INVERTING
      #define Z2_MAX_PIN X_MIN_PIN
    #elif Z2_USE_ENDSTOP == _XMAX_
      #define Z2_MAX_ENDSTOP_INVERTING X_MAX_ENDSTOP_INVERTING
      #define Z2_MAX_PIN X_MAX_PIN
    #elif Z2_USE_ENDSTOP == _YMIN_
      #define Z2_MAX_ENDSTOP_INVERTING Y_MIN_ENDSTOP_INVERTING
      #define Z2_MAX_PIN Y_MIN_PIN
    #elif Z2_USE_ENDSTOP == _YMAX_
      #define Z2_MAX_ENDSTOP_INVERTING Y_MAX_ENDSTOP_INVERTING
      #define Z2_MAX_PIN Y_MAX_PIN
    #elif Z2_USE_ENDSTOP == _ZMIN_
      #define Z2_MAX_ENDSTOP_INVERTING Z_MIN_ENDSTOP_INVERTING
      #define Z2_MAX_PIN Z_MIN_PIN
    #elif Z2_USE_ENDSTOP == _ZMAX_
      #define Z2_MAX_ENDSTOP_INVERTING Z_MAX_ENDSTOP_INVERTING
      #define Z2_MAX_PIN Z_MAX_PIN
    #else
      #define Z2_MAX_ENDSTOP_INVERTING false
    #endif
    #define Z2_MIN_ENDSTOP_INVERTING false
  #else
    #if Z2_USE_ENDSTOP == _XMIN_
      #define Z2_MIN_ENDSTOP_INVERTING X_MIN_ENDSTOP_INVERTING
      #define Z2_MIN_PIN X_MIN_PIN
    #elif Z2_USE_ENDSTOP == _XMAX_
      #define Z2_MIN_ENDSTOP_INVERTING X_MAX_ENDSTOP_INVERTING
      #define Z2_MIN_PIN X_MAX_PIN
    #elif Z2_USE_ENDSTOP == _YMIN_
      #define Z2_MIN_ENDSTOP_INVERTING Y_MIN_ENDSTOP_INVERTING
      #define Z2_MIN_PIN Y_MIN_PIN
    #elif Z2_USE_ENDSTOP == _YMAX_
      #define Z2_MIN_ENDSTOP_INVERTING Y_MAX_ENDSTOP_INVERTING
      #define Z2_MIN_PIN Y_MAX_PIN
    #elif Z2_USE_ENDSTOP == _ZMIN_
      #define Z2_MIN_ENDSTOP_INVERTING Z_MIN_ENDSTOP_INVERTING
      #define Z2_MIN_PIN Z_MIN_PIN
    #elif Z2_USE_ENDSTOP == _ZMAX_
      #define Z2_MIN_ENDSTOP_INVERTING Z_MAX_ENDSTOP_INVERTING
      #define Z2_MIN_PIN Z_MAX_PIN
    #else
      #define Z2_MIN_ENDSTOP_INVERTING false
    #endif
    #define Z2_MAX_ENDSTOP_INVERTING false
  #endif
#endif

#if ENABLED(Z_TRIPLE_ENDSTOPS)
  #if Z_HOME_DIR > 0
    #if Z3_USE_ENDSTOP == _XMIN_
      #define Z3_MAX_ENDSTOP_INVERTING X_MIN_ENDSTOP_INVERTING
      #define Z3_MAX_PIN X_MIN_PIN
    #elif Z3_USE_ENDSTOP == _XMAX_
      #define Z3_MAX_ENDSTOP_INVERTING X_MAX_ENDSTOP_INVERTING
      #define Z3_MAX_PIN X_MAX_PIN
    #elif Z3_USE_ENDSTOP == _YMIN_
      #define Z3_MAX_ENDSTOP_INVERTING Y_MIN_ENDSTOP_INVERTING
      #define Z3_MAX_PIN Y_MIN_PIN
    #elif Z3_USE_ENDSTOP == _YMAX_
      #define Z3_MAX_ENDSTOP_INVERTING Y_MAX_ENDSTOP_INVERTING
      #define Z3_MAX_PIN Y_MAX_PIN
    #elif Z3_USE_ENDSTOP == _ZMIN_
      #define Z3_MAX_ENDSTOP_INVERTING Z_MIN_ENDSTOP_INVERTING
      #define Z3_MAX_PIN Z_MIN_PIN
    #elif Z3_USE_ENDSTOP == _ZMAX_
      #define Z3_MAX_ENDSTOP_INVERTING Z_MAX_ENDSTOP_INVERTING
      #define Z3_MAX_PIN Z_MAX_PIN
    #else
      #define Z3_MAX_ENDSTOP_INVERTING false
    #endif
    #define Z3_MIN_ENDSTOP_INVERTING false
  #else
    #if Z3_USE_ENDSTOP == _XMIN_
      #define Z3_MIN_ENDSTOP_INVERTING X_MIN_ENDSTOP_INVERTING
      #define Z3_MIN_PIN X_MIN_PIN
    #elif Z3_USE_ENDSTOP == _XMAX_
      #define Z3_MIN_ENDSTOP_INVERTING X_MAX_ENDSTOP_INVERTING
      #define Z3_MIN_PIN X_MAX_PIN
    #elif Z3_USE_ENDSTOP == _YMIN_
      #define Z3_MIN_ENDSTOP_INVERTING Y_MIN_ENDSTOP_INVERTING
      #define Z3_MIN_PIN Y_MIN_PIN
    #elif Z3_USE_ENDSTOP == _YMAX_
      #define Z3_MIN_ENDSTOP_INVERTING Y_MAX_ENDSTOP_INVERTING
      #define Z3_MIN_PIN Y_MAX_PIN
    #elif Z3_USE_ENDSTOP == _ZMIN_
      #define Z3_MIN_ENDSTOP_INVERTING Z_MIN_ENDSTOP_INVERTING
      #define Z3_MIN_PIN Z_MIN_PIN
    #elif Z3_USE_ENDSTOP == _ZMAX_
      #define Z3_MIN_ENDSTOP_INVERTING Z_MAX_ENDSTOP_INVERTING
      #define Z3_MIN_PIN Z_MAX_PIN
    #else
      #define Z3_MIN_ENDSTOP_INVERTING false
    #endif
    #define Z3_MAX_ENDSTOP_INVERTING false
  #endif
#endif

// Is an endstop plug used for the Z2 endstop or the bed probe?
#define IS_Z2_OR_PROBE(A,M) ( \
     (Z_MULTI_ENDSTOPS && Z2_USE_ENDSTOP == _##A##M##_) \
  || (HAS_CUSTOM_PROBE_PIN && Z_MIN_PROBE_PIN == A##_##M##_PIN ) )

// Is an endstop plug used for the Z3 endstop or the bed probe?
#define IS_Z3_OR_PROBE(A,M) ( \
     (ENABLED(Z_TRIPLE_ENDSTOPS) && Z3_USE_ENDSTOP == _##A##M##_) \
  || (HAS_CUSTOM_PROBE_PIN && Z_MIN_PROBE_PIN == A##_##M##_PIN ) )

/**
 * Set ENDSTOPPULLUPS for active endstop switches
 */
#if ENABLED(ENDSTOPPULLUPS)
  #if ENABLED(USE_XMAX_PLUG)
    #define ENDSTOPPULLUP_XMAX
  #endif
  #if ENABLED(USE_YMAX_PLUG)
    #define ENDSTOPPULLUP_YMAX
  #endif
  #if ENABLED(USE_ZMAX_PLUG)
    #define ENDSTOPPULLUP_ZMAX
  #endif
  #if ENABLED(USE_XMIN_PLUG)
    #define ENDSTOPPULLUP_XMIN
  #endif
  #if ENABLED(USE_YMIN_PLUG)
    #define ENDSTOPPULLUP_YMIN
  #endif
  #if ENABLED(USE_ZMIN_PLUG)
    #define ENDSTOPPULLUP_ZMIN
  #endif
#endif

/**
 * Set ENDSTOPPULLDOWNS for active endstop switches
 */
#if ENABLED(ENDSTOPPULLDOWNS)
  #if ENABLED(USE_XMAX_PLUG)
    #define ENDSTOPPULLDOWN_XMAX
  #endif
  #if ENABLED(USE_YMAX_PLUG)
    #define ENDSTOPPULLDOWN_YMAX
  #endif
  #if ENABLED(USE_ZMAX_PLUG)
    #define ENDSTOPPULLDOWN_ZMAX
  #endif
  #if ENABLED(USE_XMIN_PLUG)
    #define ENDSTOPPULLDOWN_XMIN
  #endif
  #if ENABLED(USE_YMIN_PLUG)
    #define ENDSTOPPULLDOWN_YMIN
  #endif
  #if ENABLED(USE_ZMIN_PLUG)
    #define ENDSTOPPULLDOWN_ZMIN
  #endif
#endif

/**
 * Shorthand for pin tests, used wherever needed
 */

// Steppers
#define HAS_X_ENABLE      (PIN_EXISTS(X_ENABLE) || (ENABLED(SOFTWARE_DRIVER_ENABLE) && AXIS_IS_TMC(X)))
#define HAS_X_DIR         (PIN_EXISTS(X_DIR))
#define HAS_X_STEP        (PIN_EXISTS(X_STEP))
#define HAS_X_MICROSTEPS  (PIN_EXISTS(X_MS1))

#define HAS_X2_ENABLE     (PIN_EXISTS(X2_ENABLE) || (ENABLED(SOFTWARE_DRIVER_ENABLE) && AXIS_IS_TMC(X2)))
#define HAS_X2_DIR        (PIN_EXISTS(X2_DIR))
#define HAS_X2_STEP       (PIN_EXISTS(X2_STEP))
#define HAS_X2_MICROSTEPS (PIN_EXISTS(X2_MS1))

#define HAS_Y_ENABLE      (PIN_EXISTS(Y_ENABLE) || (ENABLED(SOFTWARE_DRIVER_ENABLE) && AXIS_IS_TMC(Y)))
#define HAS_Y_DIR         (PIN_EXISTS(Y_DIR))
#define HAS_Y_STEP        (PIN_EXISTS(Y_STEP))
#define HAS_Y_MICROSTEPS  (PIN_EXISTS(Y_MS1))

#define HAS_Y2_ENABLE     (PIN_EXISTS(Y2_ENABLE) || (ENABLED(SOFTWARE_DRIVER_ENABLE) && AXIS_IS_TMC(Y2)))
#define HAS_Y2_DIR        (PIN_EXISTS(Y2_DIR))
#define HAS_Y2_STEP       (PIN_EXISTS(Y2_STEP))
#define HAS_Y2_MICROSTEPS (PIN_EXISTS(Y2_MS1))

#define HAS_Z_ENABLE      (PIN_EXISTS(Z_ENABLE) || (ENABLED(SOFTWARE_DRIVER_ENABLE) && AXIS_IS_TMC(Z)))
#define HAS_Z_DIR         (PIN_EXISTS(Z_DIR))
#define HAS_Z_STEP        (PIN_EXISTS(Z_STEP))
#define HAS_Z_MICROSTEPS  (PIN_EXISTS(Z_MS1))

#define HAS_Z2_ENABLE     (PIN_EXISTS(Z2_ENABLE) || (ENABLED(SOFTWARE_DRIVER_ENABLE) && AXIS_IS_TMC(Z2)))
#define HAS_Z2_DIR        (PIN_EXISTS(Z2_DIR))
#define HAS_Z2_STEP       (PIN_EXISTS(Z2_STEP))
#define HAS_Z2_MICROSTEPS (PIN_EXISTS(Z2_MS1))

#define HAS_Z3_ENABLE     (PIN_EXISTS(Z3_ENABLE) || (ENABLED(SOFTWARE_DRIVER_ENABLE) && AXIS_IS_TMC(Z3)))
#define HAS_Z3_DIR        (PIN_EXISTS(Z3_DIR))
#define HAS_Z3_STEP       (PIN_EXISTS(Z3_STEP))
#define HAS_Z3_MICROSTEPS (PIN_EXISTS(Z3_MS1))

// Extruder steppers and solenoids
#define HAS_E0_ENABLE     (PIN_EXISTS(E0_ENABLE) || (ENABLED(SOFTWARE_DRIVER_ENABLE) && AXIS_IS_TMC(E0)))
#define HAS_E0_DIR        (PIN_EXISTS(E0_DIR))
#define HAS_E0_STEP       (PIN_EXISTS(E0_STEP))
#define HAS_E0_MICROSTEPS (PIN_EXISTS(E0_MS1))
#define HAS_SOLENOID_0    (PIN_EXISTS(SOL0))

#define HAS_E1_ENABLE     (PIN_EXISTS(E1_ENABLE) || (ENABLED(SOFTWARE_DRIVER_ENABLE) && AXIS_IS_TMC(E1)))
#define HAS_E1_DIR        (PIN_EXISTS(E1_DIR))
#define HAS_E1_STEP       (PIN_EXISTS(E1_STEP))
#define HAS_E1_MICROSTEPS (PIN_EXISTS(E1_MS1))
#define HAS_SOLENOID_1    (PIN_EXISTS(SOL1))

#define HAS_E2_ENABLE     (PIN_EXISTS(E2_ENABLE) || (ENABLED(SOFTWARE_DRIVER_ENABLE) && AXIS_IS_TMC(E2)))
#define HAS_E2_DIR        (PIN_EXISTS(E2_DIR))
#define HAS_E2_STEP       (PIN_EXISTS(E2_STEP))
#define HAS_E2_MICROSTEPS (PIN_EXISTS(E2_MS1))
#define HAS_SOLENOID_2    (PIN_EXISTS(SOL2))

#define HAS_E3_ENABLE     (PIN_EXISTS(E3_ENABLE) || (ENABLED(SOFTWARE_DRIVER_ENABLE) && AXIS_IS_TMC(E3)))
#define HAS_E3_DIR        (PIN_EXISTS(E3_DIR))
#define HAS_E3_STEP       (PIN_EXISTS(E3_STEP))
#define HAS_E3_MICROSTEPS (PIN_EXISTS(E3_MS1))
#define HAS_SOLENOID_3    (PIN_EXISTS(SOL3))

#define HAS_E4_ENABLE     (PIN_EXISTS(E4_ENABLE) || (ENABLED(SOFTWARE_DRIVER_ENABLE) && AXIS_IS_TMC(E4)))
#define HAS_E4_DIR        (PIN_EXISTS(E4_DIR))
#define HAS_E4_STEP       (PIN_EXISTS(E4_STEP))
#define HAS_E4_MICROSTEPS (PIN_EXISTS(E4_MS1))
#define HAS_SOLENOID_4    (PIN_EXISTS(SOL4))

#define HAS_E5_ENABLE     (PIN_EXISTS(E5_ENABLE) || (ENABLED(SOFTWARE_DRIVER_ENABLE) && AXIS_IS_TMC(E5)))
#define HAS_E5_DIR        (PIN_EXISTS(E5_DIR))
#define HAS_E5_STEP       (PIN_EXISTS(E5_STEP))
#define HAS_E5_MICROSTEPS (PIN_EXISTS(E5_MS1))
#define HAS_SOLENOID_5    (PIN_EXISTS(SOL5))

// Trinamic Stepper Drivers
#if HAS_TRINAMIC
  #define HAS_TMCX1X0       (HAS_DRIVER(TMC2130) || HAS_DRIVER(TMC2160) || HAS_DRIVER(TMC5130) || HAS_DRIVER(TMC5160))
  #define TMC_HAS_SPI       (HAS_TMCX1X0 || HAS_DRIVER(TMC2660))
  #define HAS_STALLGUARD    (HAS_TMCX1X0 || HAS_DRIVER(TMC2209) || HAS_DRIVER(TMC2660))
  #define HAS_STEALTHCHOP   (HAS_TMCX1X0 || HAS_TMC220x)

  #define STEALTHCHOP_ENABLED ANY(STEALTHCHOP_XY, STEALTHCHOP_Z, STEALTHCHOP_E)
  #define USE_SENSORLESS EITHER(SENSORLESS_HOMING, SENSORLESS_PROBING)
  #if (AXIS_HAS_STALLGUARD(X)  && defined(X_STALL_SENSITIVITY))
    #define X_SENSORLESS 1
  #endif
  #if (AXIS_HAS_STALLGUARD(X2) && defined(X2_STALL_SENSITIVITY))
    #define X2_SENSORLESS 1
  #endif
  #if (AXIS_HAS_STALLGUARD(Y)  && defined(Y_STALL_SENSITIVITY))
    #define Y_SENSORLESS 1
  #endif
  #if (AXIS_HAS_STALLGUARD(Y2) && defined(Y2_STALL_SENSITIVITY))
    #define Y2_SENSORLESS 1
  #endif
  #define Z_SENSORLESS  (AXIS_HAS_STALLGUARD(Z)  && defined(Z_STALL_SENSITIVITY))
  #if (AXIS_HAS_STALLGUARD(Z2) && defined(Z2_STALL_SENSITIVITY))
    #define Z2_SENSORLESS 1
  #endif
  #if (AXIS_HAS_STALLGUARD(Z3) && defined(Z3_STALL_SENSITIVITY))
    #define Z3_SENSORLESS 1
  #endif
  #if ENABLED(SPI_ENDSTOPS)
    #define X_SPI_SENSORLESS X_SENSORLESS
    #define Y_SPI_SENSORLESS Y_SENSORLESS
    #define Z_SPI_SENSORLESS Z_SENSORLESS
  #endif
#endif

#define HAS_E_STEPPER_ENABLE (HAS_E_DRIVER(TMC2660) \
  || ( E0_ENABLE_PIN != X_ENABLE_PIN && E1_ENABLE_PIN != X_ENABLE_PIN   \
    && E0_ENABLE_PIN != Y_ENABLE_PIN && E1_ENABLE_PIN != Y_ENABLE_PIN ) \
)

// Endstops and bed probe
#define _HAS_STOP(A,M) (PIN_EXISTS(A##_##M) && !IS_X2_ENDSTOP(A,M) && !IS_Y2_ENDSTOP(A,M) && !IS_Z2_OR_PROBE(A,M))
#define HAS_X_MIN _HAS_STOP(X,MIN)
#define HAS_X_MAX _HAS_STOP(X,MAX)
#define HAS_Y_MIN _HAS_STOP(Y,MIN)
#define HAS_Y_MAX _HAS_STOP(Y,MAX)
#define HAS_Z_MIN _HAS_STOP(Z,MIN)
#define HAS_Z_MAX _HAS_STOP(Z,MAX)
#define HAS_X2_MIN (PIN_EXISTS(X2_MIN))
#define HAS_X2_MAX (PIN_EXISTS(X2_MAX))
#define HAS_Y2_MIN (PIN_EXISTS(Y2_MIN))
#define HAS_Y2_MAX (PIN_EXISTS(Y2_MAX))
#define HAS_Z2_MIN (PIN_EXISTS(Z2_MIN))
#define HAS_Z2_MAX (PIN_EXISTS(Z2_MAX))
#define HAS_Z3_MIN (PIN_EXISTS(Z3_MIN))
#define HAS_Z3_MAX (PIN_EXISTS(Z3_MAX))
#define HAS_Z_MIN_PROBE_PIN (HAS_CUSTOM_PROBE_PIN && PIN_EXISTS(Z_MIN_PROBE))
#define HAS_CALIBRATION_PIN (PIN_EXISTS(CALIBRATION))

// ADC Temp Sensors (Thermistor or Thermocouple with amplifier ADC interface)
#define HAS_ADC_TEST(P) (PIN_EXISTS(TEMP_##P) && TEMP_SENSOR_##P != 0 && DISABLED(HEATER_##P##_USES_MAX6675))
#define HAS_TEMP_ADC_0 HAS_ADC_TEST(0)
#define HAS_TEMP_ADC_1 HAS_ADC_TEST(1)
#define HAS_TEMP_ADC_2 HAS_ADC_TEST(2)
#define HAS_TEMP_ADC_3 HAS_ADC_TEST(3)
#define HAS_TEMP_ADC_4 HAS_ADC_TEST(4)
#define HAS_TEMP_ADC_5 HAS_ADC_TEST(5)
#define HAS_TEMP_ADC_BED HAS_ADC_TEST(BED)
#define HAS_TEMP_ADC_CHAMBER HAS_ADC_TEST(CHAMBER)
#define HAS_TEMP_ADC_HEATBREAK HAS_ADC_TEST(HEATBREAK)
#define HAS_TEMP_ADC_BOARD HAS_ADC_TEST(BOARD)

#define HAS_TEMP_HOTEND (HAS_TEMP_ADC_0 || ENABLED(HEATER_0_USES_MAX6675))
#define HAS_TEMP_BED HAS_TEMP_ADC_BED
#define HAS_TEMP_CHAMBER HAS_TEMP_ADC_CHAMBER
#define HAS_TEMP_HEATBREAK HAS_TEMP_ADC_HEATBREAK
#define HAS_TEMP_BOARD HAS_TEMP_ADC_BOARD
#define HAS_HEATED_CHAMBER (HAS_TEMP_CHAMBER && PIN_EXISTS(HEATER_CHAMBER))
#define HAS_TEMP_HEATBREAK_CONTROL (HAS_TEMP_HEATBREAK && PIN_EXISTS(HEATER_HEATBREAK))  // For future use to control heatbreak temperature

#if ENABLED(JOYSTICK)
  #define HAS_JOY_ADC_X  PIN_EXISTS(JOY_X)
  #define HAS_JOY_ADC_Y  PIN_EXISTS(JOY_Y)
  #define HAS_JOY_ADC_Z  PIN_EXISTS(JOY_Z)
  #define HAS_JOY_ADC_EN PIN_EXISTS(JOY_EN)
#endif

// Heaters
#define HAS_HEATER_0 (PIN_EXISTS(HEATER_0))
#define HAS_HEATER_1 (PIN_EXISTS(HEATER_1))
#define HAS_HEATER_2 (PIN_EXISTS(HEATER_2))
#define HAS_HEATER_3 (PIN_EXISTS(HEATER_3))
#define HAS_HEATER_4 (PIN_EXISTS(HEATER_4))
#define HAS_HEATER_5 (PIN_EXISTS(HEATER_5))
#define HAS_HEATER_BED (PIN_EXISTS(HEATER_BED))
#define HAS_HEATER_HEATBREAK (PIN_EXISTS(HEATER_HEATBREAK))

// Shorthand for common combinations
#define HAS_HEATED_BED (HAS_TEMP_BED && HAS_HEATER_BED)
#define BED_OR_CHAMBER (HAS_HEATED_BED || HAS_TEMP_CHAMBER)
#define HAS_TEMP_SENSOR (HAS_TEMP_HOTEND || BED_OR_CHAMBER)

#if !HAS_TEMP_SENSOR
  #undef AUTO_REPORT_TEMPERATURES
#endif

// PID heating
#if !HAS_HEATED_BED
  #undef PIDTEMPBED
#endif
#define HAS_PID_HEATING EITHER(PIDTEMP, PIDTEMPBED)
#define HAS_PID_FOR_BOTH BOTH(PIDTEMP, PIDTEMPBED)

// Thermal protection
#define HAS_THERMALLY_PROTECTED_BED (HAS_HEATED_BED && ENABLED(THERMAL_PROTECTION_BED))
#if (ENABLED(THERMAL_PROTECTION_HOTENDS) && WATCH_TEMP_PERIOD > 0)
  #define WATCH_HOTENDS 1
#endif
#define WATCH_BED (HAS_THERMALLY_PROTECTED_BED && WATCH_BED_TEMP_PERIOD > 0)
#define WATCH_CHAMBER (HAS_HEATED_CHAMBER && ENABLED(THERMAL_PROTECTION_CHAMBER) && WATCH_CHAMBER_TEMP_PERIOD > 0)
#define WATCH_HEATBREAK (HAS_TEMP_HEATBREAK_CONTROL && ENABLED(THERMAL_PROTECTION_HEATBREAK) && WATCH_HEATBREAK_TEMP_PERIOD > 0)
#define WATCH_BOARD (HAS_TEMP_BOARD_CONTROL && ENABLED(THERMAL_PROTECTION_BOARD) && WATCH_BOARD_TEMP_PERIOD > 0)

// Auto fans
#define HAS_AUTO_FAN_0 (HOTENDS > 0 && PIN_EXISTS(E0_AUTO_FAN))
#define HAS_AUTO_FAN_1 (HOTENDS > 1 && PIN_EXISTS(E1_AUTO_FAN))
#define HAS_AUTO_FAN_2 (HOTENDS > 2 && PIN_EXISTS(E2_AUTO_FAN))
#define HAS_AUTO_FAN_3 (HOTENDS > 3 && PIN_EXISTS(E3_AUTO_FAN))
#define HAS_AUTO_FAN_4 (HOTENDS > 4 && PIN_EXISTS(E4_AUTO_FAN))
#define HAS_AUTO_FAN_5 (HOTENDS > 5 && PIN_EXISTS(E5_AUTO_FAN))
#define HAS_AUTO_CHAMBER_FAN (HAS_TEMP_CHAMBER && PIN_EXISTS(CHAMBER_AUTO_FAN))

#define HAS_AUTO_FAN (HAS_AUTO_FAN_0 || HAS_AUTO_FAN_1 || HAS_AUTO_FAN_2 || HAS_AUTO_FAN_3 || HAS_AUTO_FAN_4 || HAS_AUTO_FAN_5 || HAS_AUTO_CHAMBER_FAN)
#define _FANOVERLAP(A,B) (A##_AUTO_FAN_PIN == E##B##_AUTO_FAN_PIN)
#if HAS_AUTO_FAN
  #define AUTO_CHAMBER_IS_E (_FANOVERLAP(CHAMBER,0) || _FANOVERLAP(CHAMBER,1) || _FANOVERLAP(CHAMBER,2) || _FANOVERLAP(CHAMBER,3) || _FANOVERLAP(CHAMBER,4) || _FANOVERLAP(CHAMBER,5))
#endif

#if !HAS_AUTO_CHAMBER_FAN || AUTO_CHAMBER_IS_E
  #undef AUTO_POWER_CHAMBER_FAN
#endif

// Other fans
#define HAS_FAN0 (PIN_EXISTS(FAN))
#define HAS_FAN1 (PIN_EXISTS(FAN1) && CONTROLLER_FAN_PIN != FAN1_PIN && E0_AUTO_FAN_PIN != FAN1_PIN && E1_AUTO_FAN_PIN != FAN1_PIN && E2_AUTO_FAN_PIN != FAN1_PIN && E3_AUTO_FAN_PIN != FAN1_PIN && E4_AUTO_FAN_PIN != FAN1_PIN && E5_AUTO_FAN_PIN != FAN1_PIN)
#define HAS_FAN2 (PIN_EXISTS(FAN2) && CONTROLLER_FAN_PIN != FAN2_PIN && E0_AUTO_FAN_PIN != FAN2_PIN && E1_AUTO_FAN_PIN != FAN2_PIN && E2_AUTO_FAN_PIN != FAN2_PIN && E3_AUTO_FAN_PIN != FAN2_PIN && E4_AUTO_FAN_PIN != FAN2_PIN && E5_AUTO_FAN_PIN != FAN2_PIN)
#define HAS_CONTROLLER_FAN (PIN_EXISTS(CONTROLLER_FAN))

// Servos
#define HAS_SERVO_0 (PIN_EXISTS(SERVO0) && NUM_SERVOS > 0)
#define HAS_SERVO_1 (PIN_EXISTS(SERVO1) && NUM_SERVOS > 1)
#define HAS_SERVO_2 (PIN_EXISTS(SERVO2) && NUM_SERVOS > 2)
#define HAS_SERVO_3 (PIN_EXISTS(SERVO3) && NUM_SERVOS > 3)
#define HAS_SERVOS  (NUM_SERVOS > 0)

#if HAS_SERVOS && !defined(Z_PROBE_SERVO_NR)
  #define Z_PROBE_SERVO_NR -1
#endif

#define HAS_SERVO_ANGLES (EITHER(SWITCHING_EXTRUDER, SWITCHING_NOZZLE) || (HAS_Z_SERVO_PROBE && defined(Z_PROBE_SERVO_NR)))

#if !HAS_SERVO_ANGLES || ENABLED(BLTOUCH)
  #undef EDITABLE_SERVO_ANGLES
#endif

// Sensors
#define HAS_FILAMENT_WIDTH_SENSOR (PIN_EXISTS(FILWIDTH))

// User Interface
#define HAS_HOME        (PIN_EXISTS(HOME))
#define HAS_KILL        (PIN_EXISTS(KILL))
#define HAS_SUICIDE     (PIN_EXISTS(SUICIDE))
#define HAS_PHOTOGRAPH  (PIN_EXISTS(PHOTOGRAPH))
#define HAS_BUZZER      (PIN_EXISTS(BEEPER) || EITHER(LCD_USE_I2C_BUZZER, PCA9632_BUZZER))
#define USE_BEEPER      (HAS_BUZZER && DISABLED(LCD_USE_I2C_BUZZER, PCA9632_BUZZER))
#define HAS_CASE_LIGHT  (PIN_EXISTS(CASE_LIGHT) && ENABLED(CASE_LIGHT_ENABLE))

// Digital control
#define HAS_STEPPER_RESET     (PIN_EXISTS(STEPPER_RESET))
#define HAS_DIGIPOTSS         (PIN_EXISTS(DIGIPOTSS))
#define HAS_MOTOR_CURRENT_PWM ANY_PIN(MOTOR_CURRENT_PWM_X, MOTOR_CURRENT_PWM_Y, MOTOR_CURRENT_PWM_XY, MOTOR_CURRENT_PWM_Z, MOTOR_CURRENT_PWM_E)

#define HAS_MICROSTEPS (HAS_X_MICROSTEPS || HAS_X2_MICROSTEPS || HAS_Y_MICROSTEPS || HAS_Y2_MICROSTEPS || HAS_Z_MICROSTEPS || HAS_Z2_MICROSTEPS || HAS_Z3_MICROSTEPS || HAS_E0_MICROSTEPS || HAS_E1_MICROSTEPS || HAS_E2_MICROSTEPS || HAS_E3_MICROSTEPS || HAS_E4_MICROSTEPS || HAS_E5_MICROSTEPS)

#if HAS_MICROSTEPS

  // MS1 MS2 MS3 Stepper Driver Microstepping mode table
  #ifndef MICROSTEP1
    #define MICROSTEP1 LOW,LOW,LOW
  #endif
  #if ENABLED(HEROIC_STEPPER_DRIVERS)
    #ifndef MICROSTEP128
      #define MICROSTEP128 LOW,HIGH,LOW
    #endif
  #else
    #ifndef MICROSTEP2
      #define MICROSTEP2 HIGH,LOW,LOW
    #endif
    #ifndef MICROSTEP4
      #define MICROSTEP4 LOW,HIGH,LOW
    #endif
  #endif
  #ifndef MICROSTEP8
    #define MICROSTEP8 HIGH,HIGH,LOW
  #endif
  #ifdef __SAM3X8E__
    #if MB(ALLIGATOR)
      #ifndef MICROSTEP16
        #define MICROSTEP16 LOW,LOW,LOW
      #endif
      #ifndef MICROSTEP32
        #define MICROSTEP32 HIGH,HIGH,LOW
      #endif
    #else
      #ifndef MICROSTEP16
        #define MICROSTEP16 HIGH,HIGH,LOW
      #endif
    #endif
  #else
    #ifndef MICROSTEP16
      #define MICROSTEP16 HIGH,HIGH,LOW
    #endif
  #endif

  #define HAS_MICROSTEP1 defined(MICROSTEP1)
  #define HAS_MICROSTEP2 defined(MICROSTEP2)
  #define HAS_MICROSTEP4 defined(MICROSTEP4)
  #define HAS_MICROSTEP8 defined(MICROSTEP8)
  #define HAS_MICROSTEP16 defined(MICROSTEP16)
  #define HAS_MICROSTEP32 defined(MICROSTEP32)
  #define HAS_MICROSTEP64 defined(MICROSTEP64)
  #define HAS_MICROSTEP128 defined(MICROSTEP128)

#endif // HAS_MICROSTEPS

#if !HAS_TEMP_SENSOR
  #undef AUTO_REPORT_TEMPERATURES
#endif

#define HAS_AUTO_REPORTING EITHER(AUTO_REPORT_TEMPERATURES, AUTO_REPORT_SD_STATUS)

/**
 * This setting is also used by M109 when trying to calculate
 * a ballpark safe margin to prevent wait-forever situation.
 */
#ifndef EXTRUDE_MINTEMP
  #define EXTRUDE_MINTEMP 170
#endif

/**
 * Heater signal inversion defaults
 */

#if HAS_HEATER_0 && !defined(HEATER_0_INVERTING)
  #define HEATER_0_INVERTING false
#endif

#if HAS_HEATER_1 && !defined(HEATER_1_INVERTING)
  #define HEATER_1_INVERTING false
#endif

#if HAS_HEATER_2 && !defined(HEATER_2_INVERTING)
  #define HEATER_2_INVERTING false
#endif

#if HAS_HEATER_3 && !defined(HEATER_3_INVERTING)
  #define HEATER_3_INVERTING false
#endif

#if HAS_HEATER_4 && !defined(HEATER_4_INVERTING)
  #define HEATER_4_INVERTING false
#endif

#if HAS_HEATER_5 && !defined(HEATER_5_INVERTING)
  #define HEATER_5_INVERTING false
#endif

/**
 * Helper Macros for heaters and extruder fan
 */

#define WRITE_HEATER_0P(v) WRITE(HEATER_0_PIN, (v) ^ HEATER_0_INVERTING)
#if HOTENDS > 1 || ENABLED(HEATERS_PARALLEL)
  #define WRITE_HEATER_1(v) WRITE(HEATER_1_PIN, (v) ^ HEATER_1_INVERTING)
  #if HOTENDS > 2
    #define WRITE_HEATER_2(v) WRITE(HEATER_2_PIN, (v) ^ HEATER_2_INVERTING)
    #if HOTENDS > 3
      #define WRITE_HEATER_3(v) WRITE(HEATER_3_PIN, (v) ^ HEATER_3_INVERTING)
      #if HOTENDS > 4
        #define WRITE_HEATER_4(v) WRITE(HEATER_4_PIN, (v) ^ HEATER_4_INVERTING)
        #if HOTENDS > 5
          #define WRITE_HEATER_5(v) WRITE(HEATER_5_PIN, (v) ^ HEATER_5_INVERTING)
        #endif // HOTENDS > 5
      #endif // HOTENDS > 4
    #endif // HOTENDS > 3
  #endif // HOTENDS > 2
#endif // HOTENDS > 1
#if ENABLED(HEATERS_PARALLEL)
  #define WRITE_HEATER_0(v) { WRITE_HEATER_0P(v); WRITE_HEATER_1(v); }
#else
  #define WRITE_HEATER_0(v) WRITE_HEATER_0P(v)
#endif

#ifndef MIN_POWER
  #define MIN_POWER 0
#endif

/**
 * Heated bed requires settings
 */
#if HAS_HEATED_BED
  #ifndef MIN_BED_POWER
    #define MIN_BED_POWER 0
  #endif
  #ifndef MAX_BED_POWER
    #define MAX_BED_POWER 255
  #endif
  #ifndef HEATER_BED_INVERTING
    #define HEATER_BED_INVERTING false
  #endif
  #define WRITE_HEATER_BED(v) WRITE(HEATER_BED_PIN, (v) ^ HEATER_BED_INVERTING)
#endif

/**
 * Heated chamber requires settings
 */
#if HAS_HEATED_CHAMBER
  #ifndef MAX_CHAMBER_POWER
    #define MAX_CHAMBER_POWER 255
  #endif
  #ifndef HEATER_CHAMBER_INVERTING
    #define HEATER_CHAMBER_INVERTING false
  #endif
  #define WRITE_HEATER_CHAMBER(v) WRITE(HEATER_CHAMBER_PIN, (v) ^ HEATER_CHAMBER_INVERTING)
#endif

/**
 * Heated chamber requires settings
 */
#if HAS_TEMP_HEATBREAK_CONTROL
  #ifndef MIN_HEATBREAK_POWER
    #define MIN_HEATBREAK_POWER 0
  #endif
  #ifndef MAX_HEATBREAK_POWER
    #define MAX_HEATBREAK_POWER 255
  #endif
  #ifndef HEATER_HEATBREAK_INVERTING
    #define HEATER_HEATBREAK_INVERTING true
  #endif
  #define WRITE_HEATER_HEATBREAK(v) WRITE(HEATER_HEATBREAK_PIN, (v) ^ HEATER_HEATBREAK_INVERTING)
#endif

/**
 * Up to 3 PWM fans
 */
#ifndef FAN_INVERTING
  #define FAN_INVERTING false
#endif

#if HAS_FAN2
  #define FAN_COUNT 3
#elif HAS_FAN1
  #define FAN_COUNT 2
#elif HAS_FAN0
  #define FAN_COUNT 1
#else
  #define FAN_COUNT 0
#endif

#if FAN_COUNT > 0
  #define WRITE_FAN(n, v) WRITE(FAN##n##_PIN, (v) ^ FAN_INVERTING)
#endif

/**
 * Part Cooling fan multipliexer
 */
#define HAS_FANMUX PIN_EXISTS(FANMUX0)

/**
 * MIN/MAX fan PWM scaling
 */
#ifndef FAN_MIN_PWM
  #define FAN_MIN_PWM 0
#endif
#ifndef FAN_MAX_PWM
  #define FAN_MAX_PWM 255
#endif
#if FAN_MIN_PWM < 0 || FAN_MIN_PWM > 255
  #error "FAN_MIN_PWM must be a value from 0 to 255."
#elif FAN_MAX_PWM < 0 || FAN_MAX_PWM > 255
  #error "FAN_MAX_PWM must be a value from 0 to 255."
#elif FAN_MIN_PWM > FAN_MAX_PWM
  #error "FAN_MIN_PWM must be less than or equal to FAN_MAX_PWM."
#endif

/**
 * FAST PWM FAN Settings
 */
#if ENABLED(FAST_PWM_FAN) && !defined(FAST_PWM_FAN_FREQUENCY)
  #define FAST_PWM_FAN_FREQUENCY ((F_CPU) / (2 * 255 * 1)) // Fan frequency default
#endif

/**
 * MIN/MAX case light PWM scaling
 */
#if HAS_CASE_LIGHT
  #ifndef CASE_LIGHT_MAX_PWM
    #define CASE_LIGHT_MAX_PWM 255
  #elif !WITHIN(CASE_LIGHT_MAX_PWM, 1, 255)
    #error "CASE_LIGHT_MAX_PWM must be a value from 1 to 255."
  #endif
#endif

/**
 * Bed Probe dependencies
 */
#if HAS_BED_PROBE
  #if ENABLED(ENDSTOPPULLUPS) && HAS_Z_MIN_PROBE_PIN
    #define ENDSTOPPULLUP_ZMIN_PROBE
  #endif
  #ifndef Z_PROBE_OFFSET_RANGE_MIN
    #define Z_PROBE_OFFSET_RANGE_MIN -20
  #endif
  #ifndef Z_PROBE_OFFSET_RANGE_MAX
    #define Z_PROBE_OFFSET_RANGE_MAX 20
  #endif
  #ifndef XY_PROBE_SPEED
    #ifdef HOMING_FEEDRATE_XY
      #define XY_PROBE_SPEED HOMING_FEEDRATE_XY
    #else
      #define XY_PROBE_SPEED 4000
    #endif
  #endif
  #ifndef XY_PROBE_SPEED_INITIAL
    #define XY_PROBE_SPEED_INITIAL XY_PROBE_SPEED
  #endif
#else
  #undef NOZZLE_TO_PROBE_OFFSET
#endif

/**
 * XYZ Bed Skew Correction
 */
#if ENABLED(SKEW_CORRECTION)
  #define SKEW_FACTOR_MIN -1
  #define SKEW_FACTOR_MAX 1

  #define _GET_SIDE(a,b,c) (SQRT(2*sq(a)+2*sq(b)-4*sq(c))*0.5)
  #define _SKEW_SIDE(a,b,c) tan(M_PI*0.5-acos((sq(a)-sq(b)-sq(c))/(2*c*b)))
  #define _SKEW_FACTOR(a,b,c) _SKEW_SIDE(float(a),_GET_SIDE(float(a),float(b),float(c)),float(c))

  #ifndef XY_SKEW_FACTOR
    #if defined(XY_DIAG_AC) && defined(XY_DIAG_BD) && defined(XY_SIDE_AD)
      #define XY_SKEW_FACTOR _SKEW_FACTOR(XY_DIAG_AC, XY_DIAG_BD, XY_SIDE_AD)
    #else
      #define XY_SKEW_FACTOR 0.0
    #endif
  #endif
  #ifndef XZ_SKEW_FACTOR
    #if defined(XY_SIDE_AD) && !defined(XZ_SIDE_AD)
      #define XZ_SIDE_AD XY_SIDE_AD
    #endif
    #if defined(XZ_DIAG_AC) && defined(XZ_DIAG_BD) && defined(XZ_SIDE_AD)
      #define XZ_SKEW_FACTOR _SKEW_FACTOR(XZ_DIAG_AC, XZ_DIAG_BD, XZ_SIDE_AD)
    #else
      #define XZ_SKEW_FACTOR 0.0
    #endif
  #endif
  #ifndef YZ_SKEW_FACTOR
    #if defined(YZ_DIAG_AC) && defined(YZ_DIAG_BD) && defined(YZ_SIDE_AD)
      #define YZ_SKEW_FACTOR _SKEW_FACTOR(YZ_DIAG_AC, YZ_DIAG_BD, YZ_SIDE_AD)
    #else
      #define YZ_SKEW_FACTOR 0.0
    #endif
  #endif
#endif // SKEW_CORRECTION

/**
 * Set granular options based on the specific type of leveling
 */
#define UBL_SEGMENTED   (ENABLED(AUTO_BED_LEVELING_UBL) && ANY(SEGMENT_LEVELED_MOVES, DELTA))
#define ABL_PLANAR      EITHER(AUTO_BED_LEVELING_LINEAR, AUTO_BED_LEVELING_3POINT)
#define ABL_GRID        EITHER(AUTO_BED_LEVELING_LINEAR, AUTO_BED_LEVELING_BILINEAR)
#define HAS_ABL_NOT_UBL (ABL_PLANAR || ABL_GRID)
#define HAS_ABL_OR_UBL  (HAS_ABL_NOT_UBL || ENABLED(AUTO_BED_LEVELING_UBL))
#if (HAS_ABL_OR_UBL || ENABLED(MESH_BED_LEVELING))
  #define HAS_LEVELING 1
#endif
#define HAS_AUTOLEVEL   (HAS_ABL_OR_UBL && DISABLED(PROBE_MANUALLY))
#define HAS_MESH        ANY(AUTO_BED_LEVELING_BILINEAR, AUTO_BED_LEVELING_UBL, MESH_BED_LEVELING)
#define PLANNER_LEVELING      (HAS_LEVELING && DISABLED(AUTO_BED_LEVELING_UBL))
#define HAS_PROBING_PROCEDURE (HAS_ABL_OR_UBL || ENABLED(Z_MIN_PROBE_REPEATABILITY_TEST))
#if (ENABLED(FWRETRACT) || HAS_LEVELING || ENABLED(SKEW_CORRECTION))
#define HAS_POSITION_MODIFIERS 1
#endif

#if ENABLED(AUTO_BED_LEVELING_UBL)
  #undef LCD_BED_LEVELING
#endif

/**
 * Heater, Fan, and Probe interactions
 */
#if FAN_COUNT == 0
  #undef PROBING_FANS_OFF
  #undef ADAPTIVE_FAN_SLOWING
  #undef NO_FAN_SLOWING_IN_PID_TUNING
#endif

#define QUIET_PROBING (HAS_BED_PROBE && (EITHER(PROBING_HEATERS_OFF, PROBING_FANS_OFF) || DELAY_BEFORE_PROBING > 0))
#define HEATER_IDLE_HANDLER ANY(ADVANCED_PAUSE_FEATURE, PROBING_HEATERS_OFF, WATCH_HOTENDS)

#if ENABLED(ADVANCED_PAUSE_FEATURE) && !defined(FILAMENT_CHANGE_SLOW_LOAD_LENGTH)
  #define FILAMENT_CHANGE_SLOW_LOAD_LENGTH 0
#endif

#if EXTRUDERS > 1 && !defined(TOOLCHANGE_FIL_EXTRA_PRIME)
  #define TOOLCHANGE_FIL_EXTRA_PRIME 0
#endif

/**
 * Only constrain Z on DELTA / SCARA machines
 */
#if IS_KINEMATIC
  #undef MIN_SOFTWARE_ENDSTOP_X
  #undef MIN_SOFTWARE_ENDSTOP_Y
  #undef MAX_SOFTWARE_ENDSTOP_X
  #undef MAX_SOFTWARE_ENDSTOP_Y
#endif

/**
 * Bed Probing rectangular bounds
 * These can be further constrained in code for Delta and SCARA
 */
#ifndef MIN_PROBE_EDGE
  #define MIN_PROBE_EDGE 0
#endif
#ifndef MIN_PROBE_EDGE_LEFT
  #define MIN_PROBE_EDGE_LEFT MIN_PROBE_EDGE
#endif
#ifndef MIN_PROBE_EDGE_RIGHT
  #define MIN_PROBE_EDGE_RIGHT MIN_PROBE_EDGE
#endif
#ifndef MIN_PROBE_EDGE_FRONT
  #define MIN_PROBE_EDGE_FRONT MIN_PROBE_EDGE
#endif
#ifndef MIN_PROBE_EDGE_BACK
  #define MIN_PROBE_EDGE_BACK MIN_PROBE_EDGE
#endif

#ifndef NOZZLE_TO_PROBE_OFFSET
  #define NOZZLE_TO_PROBE_OFFSET { 0, 0, 0 }
#endif

#if ENABLED(DELTA)
  /**
   * Delta radius/rod trimmers/angle trimmers
   */
  #define _PROBE_RADIUS (DELTA_PRINTABLE_RADIUS - (MIN_PROBE_EDGE))
  #ifndef DELTA_CALIBRATION_RADIUS
    #ifdef NOZZLE_TO_PROBE_OFFSET
      #define DELTA_CALIBRATION_RADIUS (DELTA_PRINTABLE_RADIUS - _MAX(ABS(nozzle_to_probe_offset.x), ABS(nozzle_to_probe_offset.y), ABS(MIN_PROBE_EDGE)))
    #else
      #define DELTA_CALIBRATION_RADIUS _PROBE_RADIUS
    #endif
  #endif
  #ifndef DELTA_ENDSTOP_ADJ
    #define DELTA_ENDSTOP_ADJ { 0, 0, 0 }
  #endif
  #ifndef DELTA_TOWER_ANGLE_TRIM
    #define DELTA_TOWER_ANGLE_TRIM { 0, 0, 0 }
  #endif
  #ifndef DELTA_RADIUS_TRIM_TOWER
    #define DELTA_RADIUS_TRIM_TOWER { 0, 0, 0 }
  #endif
  #ifndef DELTA_DIAGONAL_ROD_TRIM_TOWER
    #define DELTA_DIAGONAL_ROD_TRIM_TOWER { 0, 0, 0 }
  #endif

  // Probing points may be verified at compile time within the radius
  // using static_assert(HYPOT2(X2-X1,Y2-Y1)<=sq(DELTA_PRINTABLE_RADIUS),"bad probe point!")
  // so that may be added to SanityCheck.h in the future.
  #define PROBE_X_MIN (X_CENTER - (_PROBE_RADIUS))
  #define PROBE_Y_MIN (Y_CENTER - (_PROBE_RADIUS))
  #define PROBE_X_MAX (X_CENTER + _PROBE_RADIUS)
  #define PROBE_Y_MAX (Y_CENTER + _PROBE_RADIUS)

#elif IS_SCARA

  #define SCARA_PRINTABLE_RADIUS (SCARA_LINKAGE_1 + SCARA_LINKAGE_2)
  #define _PROBE_RADIUS (SCARA_PRINTABLE_RADIUS - (MIN_PROBE_EDGE))
  #define PROBE_X_MIN (X_CENTER - (SCARA_PRINTABLE_RADIUS) + MIN_PROBE_EDGE_LEFT)
  #define PROBE_Y_MIN (Y_CENTER - (SCARA_PRINTABLE_RADIUS) + MIN_PROBE_EDGE_FRONT)
  #define PROBE_X_MAX (X_CENTER +  SCARA_PRINTABLE_RADIUS - (MIN_PROBE_EDGE_RIGHT))
  #define PROBE_Y_MAX (Y_CENTER +  SCARA_PRINTABLE_RADIUS - (MIN_PROBE_EDGE_BACK))

#endif

#if ENABLED(SEGMENT_LEVELED_MOVES) && !defined(LEVELED_SEGMENT_LENGTH)
  #define LEVELED_SEGMENT_LENGTH 5
#endif

/**
 * Default mesh area is an area with an inset margin on the print area.
 */
#if HAS_LEVELING
  #if IS_KINEMATIC
    // Probing points may be verified at compile time within the radius
    // using static_assert(HYPOT2(X2-X1,Y2-Y1)<=sq(DELTA_PRINTABLE_RADIUS),"bad probe point!")
    // so that may be added to SanityCheck.h in the future.
    #define _MESH_MIN_X (X_MIN_BED + MESH_INSET)
    #define _MESH_MIN_Y (Y_MIN_BED + MESH_INSET)
    #define _MESH_MAX_X (X_MAX_BED - (MESH_INSET))
    #define _MESH_MAX_Y (Y_MAX_BED - (MESH_INSET))
  #else
    // Boundaries for Cartesian probing based on set limits
    #if EITHER(MESH_BED_LEVELING, AUTO_BED_LEVELING_UBL)
      #define _MESH_MIN_X (_MAX(X_MIN_BED + MESH_INSET, X_MIN_POS))  // UBL is careful not to probe off the bed.  It does not
      #define _MESH_MIN_Y (_MAX(Y_MIN_BED + MESH_INSET, Y_MIN_POS))  // need NOZZLE_TO_PROBE_OFFSET in the mesh dimensions
      #define _MESH_MAX_X (_MIN(X_MAX_BED - (MESH_INSET), X_MAX_POS))
      #define _MESH_MAX_Y (_MIN(Y_MAX_BED - (MESH_INSET), Y_MAX_POS))
    #else
      #define _MESH_MIN_X (_MAX(X_MIN_BED + MESH_INSET, X_MIN_POS + nozzle_to_probe_offset.x))
      #define _MESH_MIN_Y (_MAX(Y_MIN_BED + MESH_INSET, Y_MIN_POS + nozzle_to_probe_offset.y))
      #define _MESH_MAX_X (_MIN(X_MAX_BED - (MESH_INSET), X_MAX_POS + nozzle_to_probe_offset.x))
      #define _MESH_MAX_Y (_MIN(Y_MAX_BED - (MESH_INSET), Y_MAX_POS + nozzle_to_probe_offset.y))
    #endif
  #endif

  // These may be overridden in Configuration.h if a smaller area is desired
  #ifndef MESH_MIN_X
    #define MESH_MIN_X _MESH_MIN_X
  #endif
  #ifndef MESH_MIN_Y
    #define MESH_MIN_Y _MESH_MIN_Y
  #endif
  #ifndef MESH_MAX_X
    #define MESH_MAX_X _MESH_MAX_X
  #endif
  #ifndef MESH_MAX_Y
    #define MESH_MAX_Y _MESH_MAX_Y
  #endif

#endif // MESH_BED_LEVELING || AUTO_BED_LEVELING_UBL

#if ALL(PROBE_PT_1_X, PROBE_PT_2_X, PROBE_PT_3_X, PROBE_PT_1_Y, PROBE_PT_2_Y, PROBE_PT_3_Y)
  #define HAS_FIXED_3POINT;
#endif

#if EITHER(AUTO_BED_LEVELING_UBL, AUTO_BED_LEVELING_3POINT) && IS_KINEMATIC
    #define HAS_FIXED_3POINT
    #define SIN0    0.0
    #define SIN120  0.866025
    #define SIN240 -0.866025
    #define COS0    1.0
    #define COS120 -0.5
    #define COS240 -0.5
    #ifndef PROBE_PT_1_X
      #define PROBE_PT_1_X (X_CENTER + (_PROBE_RADIUS) * COS0)
    #endif
    #ifndef PROBE_PT_1_Y
      #define PROBE_PT_1_Y (Y_CENTER + (_PROBE_RADIUS) * SIN0)
    #endif
    #ifndef PROBE_PT_2_X
      #define PROBE_PT_2_X (X_CENTER + (_PROBE_RADIUS) * COS120)
    #endif
    #ifndef PROBE_PT_2_Y
      #define PROBE_PT_2_Y (Y_CENTER + (_PROBE_RADIUS) * SIN120)
    #endif
    #ifndef PROBE_PT_3_X
      #define PROBE_PT_3_X (X_CENTER + (_PROBE_RADIUS) * COS240)
    #endif
    #ifndef PROBE_PT_3_Y
      #define PROBE_PT_3_Y (Y_CENTER + (_PROBE_RADIUS) * SIN240)
    #endif
#endif

/**
 * Buzzer/Speaker
 */
#if ENABLED(LCD_USE_I2C_BUZZER)
  #ifndef LCD_FEEDBACK_FREQUENCY_HZ
    #define LCD_FEEDBACK_FREQUENCY_HZ 1000
  #endif
  #ifndef LCD_FEEDBACK_FREQUENCY_DURATION_MS
    #define LCD_FEEDBACK_FREQUENCY_DURATION_MS 100
  #endif
#elif HAS_BUZZER
  #ifndef LCD_FEEDBACK_FREQUENCY_HZ
    #define LCD_FEEDBACK_FREQUENCY_HZ 5000
  #endif
  #ifndef LCD_FEEDBACK_FREQUENCY_DURATION_MS
    #define LCD_FEEDBACK_FREQUENCY_DURATION_MS 2
  #endif
#endif

/**
 * Make sure DOGLCD_SCK and DOGLCD_MOSI are defined.
 */
#if HAS_GRAPHICAL_LCD
  #ifndef DOGLCD_SCK
    #define DOGLCD_SCK  SCK_PIN
  #endif
  #ifndef DOGLCD_MOSI
    #define DOGLCD_MOSI MOSI_PIN
  #endif
#endif

/**
 * Z_HOMING_HEIGHT / Z_CLEARANCE_BETWEEN_PROBES
 */
#ifndef Z_HOMING_HEIGHT
  #ifndef Z_CLEARANCE_BETWEEN_PROBES
    #define Z_HOMING_HEIGHT 0
  #else
    #define Z_HOMING_HEIGHT Z_CLEARANCE_BETWEEN_PROBES
  #endif
#endif

#if PROBE_SELECTED
  #ifndef Z_CLEARANCE_BETWEEN_PROBES
    #define Z_CLEARANCE_BETWEEN_PROBES Z_HOMING_HEIGHT
  #endif
  #define MANUAL_PROBE_HEIGHT std::max(Z_CLEARANCE_BETWEEN_PROBES, Z_HOMING_HEIGHT)
  #ifndef Z_CLEARANCE_MULTI_PROBE
    #define Z_CLEARANCE_MULTI_PROBE Z_CLEARANCE_BETWEEN_PROBES
  #endif
  #if ENABLED(BLTOUCH) && !defined(BLTOUCH_DELAY)
    #define BLTOUCH_DELAY 500
  #endif
#endif

#ifndef __SAM3X8E__ //todo: hal: broken hal encapsulation
  #undef UI_VOLTAGE_LEVEL
  #undef RADDS_DISPLAY
  #undef MOTOR_CURRENT
#endif

// Updated G92 behavior shifts the workspace
#define HAS_POSITION_SHIFT DISABLED(NO_WORKSPACE_OFFSETS)
// The home offset also shifts the coordinate space
#define HAS_HOME_OFFSET (DISABLED(NO_WORKSPACE_OFFSETS) && IS_CARTESIAN)
// The SCARA home offset applies only on G28
#define HAS_SCARA_OFFSET (DISABLED(NO_WORKSPACE_OFFSETS) && IS_SCARA)
// Cumulative offset to workspace to save some calculation
#define HAS_WORKSPACE_OFFSET (HAS_POSITION_SHIFT && HAS_HOME_OFFSET)
// M206 sets the home offset for Cartesian machines
#define HAS_M206_COMMAND (HAS_HOME_OFFSET && !IS_SCARA)

// LCD timeout to status screen default is 15s
#ifndef LCD_TIMEOUT_TO_STATUS
  #define LCD_TIMEOUT_TO_STATUS 15000
#endif

// Add commands that need sub-codes to this list
#define USE_GCODE_SUBCODES ANY(G38_PROBE_TARGET, CNC_COORDINATE_SYSTEMS, POWER_LOSS_RECOVERY, PRINT_CHECKING_Q_CMDS)

// Parking Extruder
#if ENABLED(PARKING_EXTRUDER)
  #ifndef PARKING_EXTRUDER_GRAB_DISTANCE
    #define PARKING_EXTRUDER_GRAB_DISTANCE 0
  #endif
  #ifndef PARKING_EXTRUDER_SOLENOIDS_PINS_ACTIVE
    #define PARKING_EXTRUDER_SOLENOIDS_PINS_ACTIVE HIGH
  #endif
#endif

// Number of VFAT entries used. Each entry has 13 UTF-16 characters
#if ENABLED(SCROLL_LONG_FILENAMES)
  #define MAX_VFAT_ENTRIES (5)
#else
  #define MAX_VFAT_ENTRIES (2)
#endif

// Set defaults for unspecified LED user colors
#if ENABLED(LED_CONTROL_MENU)
  #ifndef LED_USER_PRESET_RED
    #define LED_USER_PRESET_RED       255
  #endif
  #ifndef LED_USER_PRESET_GREEN
    #define LED_USER_PRESET_GREEN     255
  #endif
  #ifndef LED_USER_PRESET_BLUE
    #define LED_USER_PRESET_BLUE      255
  #endif
  #ifndef LED_USER_PRESET_WHITE
    #define LED_USER_PRESET_WHITE     0
  #endif
  #ifndef LED_USER_PRESET_BRIGHTNESS
    #ifdef NEOPIXEL_BRIGHTNESS
      #define LED_USER_PRESET_BRIGHTNESS NEOPIXEL_BRIGHTNESS
    #else
      #define LED_USER_PRESET_BRIGHTNESS 255
    #endif
  #endif
#endif

// Nozzle park for Delta
#if BOTH(NOZZLE_PARK_FEATURE, DELTA)
  #undef NOZZLE_PARK_Z_FEEDRATE
  #define NOZZLE_PARK_Z_FEEDRATE NOZZLE_PARK_XY_FEEDRATE
#endif

// Force SDCARD_SORT_ALPHA to be enabled for Graphical LCD on LPC1768
// on boards where SD card and LCD display share the same SPI bus
// because of a bug in the shared SPI implementation. (See #8122)
#if defined(TARGET_LPC1768) && ENABLED(REPRAP_DISCOUNT_FULL_GRAPHIC_SMART_CONTROLLER) && (SCK_PIN == LCD_PINS_D4)
  #define SDCARD_SORT_ALPHA         // Keeps one directory level in RAM. Changing
                                    // directory levels still glitches the screen,
                                    // but the following LCD update cleans it up.
  #undef SDSORT_LIMIT
  #undef SDSORT_USES_RAM
  #undef SDSORT_USES_STACK
  #undef SDSORT_CACHE_NAMES
  #define SDSORT_LIMIT       64
  #define SDSORT_USES_RAM    true
  #define SDSORT_USES_STACK  false
  #define SDSORT_CACHE_NAMES true
  #ifndef FOLDER_SORTING
    #define FOLDER_SORTING     -1
  #endif
  #ifndef SDSORT_GCODE
    #define SDSORT_GCODE       false
  #endif
  #ifndef SDSORT_DYNAMIC_RAM
    #define SDSORT_DYNAMIC_RAM false
  #endif
  #ifndef SDSORT_CACHE_VFATS
    #define SDSORT_CACHE_VFATS 2
  #endif
#endif

// Defined here to catch the above defines
#if ENABLED(SDCARD_SORT_ALPHA)
  #define HAS_FOLDER_SORTING (FOLDER_SORTING || ENABLED(SDSORT_GCODE))
#endif

// If platform requires early initialization of watchdog to properly boot
#define EARLY_WATCHDOG (ENABLED(USE_WATCHDOG) && defined(ARDUINO_ARCH_SAM))

#if ENABLED(Z_TRIPLE_STEPPER_DRIVERS)
  #define Z_STEPPER_COUNT 3
#elif ENABLED(Z_DUAL_STEPPER_DRIVERS)
  #define Z_STEPPER_COUNT 2
#else
  #define Z_STEPPER_COUNT 1
#endif

#if HAS_SPI_LCD
  // Get LCD character width/height, which may be overridden by pins, configs, etc.
  #ifndef LCD_WIDTH
    #if HAS_GRAPHICAL_LCD
      #define LCD_WIDTH 21
    #elif ENABLED(ULTIPANEL)
      #define LCD_WIDTH 20
    #else
      #define LCD_WIDTH 16
    #endif
  #endif
  #ifndef LCD_HEIGHT
    #if HAS_GRAPHICAL_LCD
      #define LCD_HEIGHT 5
    #elif ENABLED(ULTIPANEL)
      #define LCD_HEIGHT 4
    #else
      #define LCD_HEIGHT 2
    #endif
  #endif
#endif

#if ENABLED(SDSUPPORT)
  #if SD_CONNECTION_IS(ONBOARD) && DISABLED(NO_SD_HOST_DRIVE)
    //
    // The external SD card is not used. Hardware SPI is used to access the card.
    // When sharing the SD card with a PC we want the menu options to
    // mount/unmount the card and refresh it. So we disable card detect.
    //
    #undef SD_DETECT_PIN
    #define SHARED_SD_CARD
  #endif
  #if DISABLED(SHARED_SD_CARD)
    #define INIT_SDCARD_ON_BOOT
  #endif
#endif

#if !NUM_SERIAL
  #undef BAUD_RATE_GCODE
#endif

#if ENABLED(Z_STEPPER_ALIGN_KNOWN_STEPPER_POSITIONS)
  #undef Z_STEPPER_ALIGN_AMP
#endif
#ifndef Z_STEPPER_ALIGN_AMP
  #define Z_STEPPER_ALIGN_AMP 1.0
#endif
