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
#pragma once

// clang-format off

#include <buddy/filename_defs.h>

/**
 * Configuration_adv.h
 *
 * Advanced settings.
 * Only change these if you know exactly what you're doing.
 * Some of these settings can damage your printer if improperly set!
 *
 * Basic settings can be found in Configuration.h
 *
 */
#define CONFIGURATION_ADV_H_VERSION 020000

/**
 * Cancel Objects
 *
 * Implement M486 to allow Marlin to skip objects
 */
#define CANCEL_OBJECTS
#ifdef CANCEL_OBJECTS
  #define CANCEL_OBJECTS_REPORTING // Emit the current object as a status message
#endif

// @section temperature

//===========================================================================
//=============================Thermal Settings  ============================
//===========================================================================

//
// Hephestos 2 24V heated bed upgrade kit.
// https://store.bq.com/en/heated-bed-kit-hephestos2
//
//#define HEPHESTOS2_HEATED_BED_KIT
#if ENABLED(HEPHESTOS2_HEATED_BED_KIT)
    #undef TEMP_SENSOR_BED
    #define TEMP_SENSOR_BED 70
    #define HEATER_BED_INVERTING true
#endif

#if DISABLED(PIDTEMPBED)
    #define BED_CHECK_INTERVAL 5000 // ms between checks in bang-bang control
    #if ENABLED(BED_LIMIT_SWITCHING)
        #define BED_HYSTERESIS 2 // Only disable heating if T>target+BED_HYSTERESIS and enable heating if T>target-BED_HYSTERESIS
    #endif
#endif

/**
 * Thermal Protection provides additional protection to your printer from damage
 * and fire. Marlin always includes safe min and max temperature ranges which
 * protect against a broken or disconnected thermistor wire.
 *
 * The issue: If a thermistor falls out, it will report the much lower
 * temperature of the air in the room, and the the firmware will keep
 * the heater on.
 *
 * The solution: Once the temperature reaches the target, start observing.
 * If the temperature stays too far below the target (hysteresis) for too
 * long (period), the firmware will halt the machine as a safety precaution.
 *
 * If you get false positives for "Thermal Runaway", increase
 * THERMAL_PROTECTION_HYSTERESIS and/or THERMAL_PROTECTION_PERIOD
 */
#if ENABLED(THERMAL_PROTECTION_HOTENDS)
    #define THERMAL_PROTECTION_PERIOD 80 //40        // Seconds
    #define THERMAL_PROTECTION_HYSTERESIS 15 //4     // Degrees Celsius

    //#define ADAPTIVE_FAN_SLOWING              // Slow part cooling fan if temperature drops
    #if BOTH(ADAPTIVE_FAN_SLOWING, PIDTEMP)
    //#define NO_FAN_SLOWING_IN_PID_TUNING    // Don't slow fan speed during M303
    #endif

    /**
   * Whenever an M104, M109, or M303 increases the target temperature, the
   * firmware will wait for the WATCH_TEMP_PERIOD to expire. If the temperature
   * hasn't increased by WATCH_TEMP_INCREASE degrees, the machine is halted and
   * requires a hard reset. This test restarts with any M104/M109/M303, but only
   * if the current temperature is far enough below the target for a reliable
   * test.
   *
   * If you get false positives for "Heating failed", increase WATCH_TEMP_PERIOD
   * and/or decrease WATCH_TEMP_INCREASE. WATCH_TEMP_INCREASE should not be set
   * below 2.
   */
    #define WATCH_TEMP_PERIOD 20 // Seconds
    #define WATCH_TEMP_INCREASE 2 // Degrees Celsius
#endif

/**
 * Thermal Protection parameters for the bed are just as above for hotends.
 */
#if ENABLED(THERMAL_PROTECTION_BED)
    #define THERMAL_PROTECTION_BED_PERIOD 20 // Seconds
    #define THERMAL_PROTECTION_BED_HYSTERESIS 10 // Degrees Celsius

    /**
   * As described above, except for the bed (M140/M190/M303).
   */
    #define WATCH_BED_TEMP_PERIOD 120 // Seconds
    #define WATCH_BED_TEMP_INCREASE 2 // Degrees Celsius

#endif

/**
 * Thermal Protection parameters for the heated chamber.
 */
#if ENABLED(THERMAL_PROTECTION_CHAMBER)
    #define THERMAL_PROTECTION_CHAMBER_PERIOD 20 // Seconds
    #define THERMAL_PROTECTION_CHAMBER_HYSTERESIS 2 // Degrees Celsius

    /**
   * Heated chamber watch settings (M141/M191).
   */
    #define WATCH_CHAMBER_TEMP_PERIOD 60 // Seconds
    #define WATCH_CHAMBER_TEMP_INCREASE 2 // Degrees Celsius
#endif

#if ENABLED(PIDTEMP)
    // this adds an experimental additional term to the heating power, proportional to the extrusion speed.
    // if Kc is chosen well, the additional required power due to increased melting should be compensated.
    //#define PID_EXTRUSION_SCALING
    #if ENABLED(PID_EXTRUSION_SCALING)
        /**
         * Increase in PWM duty cycle needed to to extrude 1 mm^2 per second
         * of filament if extruder temperature is 1 Kelvin above ambient
         * temperature [s*K^-1*mm^-2]
         */
        #define DEFAULT_Kc 0.009517f
    #endif
#endif

/**
 * Automatic Temperature:
 * The hotend target temperature is calculated by all the buffered lines of gcode.
 * The maximum buffered steps/sec of the extruder motor is called "se".
 * Start autotemp mode with M109 S<mintemp> B<maxtemp> F<factor>
 * The target temperature is set to mintemp+factor*se[steps/sec] and is limited by
 * mintemp and maxtemp. Turn this off by executing M109 without F*
 * Also, if the temperature is set to a value below mintemp, it will not be changed by autotemp.
 * On an Ultimaker, some initial testing worked with M109 S215 B260 F1 in the start.gcode
 */
//#define AUTOTEMP
#if ENABLED(AUTOTEMP)
    #define AUTOTEMP_OLDWEIGHT 0.98
#endif

// Show extra position information in M114
//#define M114_DETAIL

// Show Temperature ADC value
// Enable for M105 to include ADC values read from temperature sensors.
//#define SHOW_TEMP_ADC_VALUES

/**
 * High Temperature Thermistor Support
 *
 * Thermistors able to support high temperature tend to have a hard time getting
 * good readings at room and lower temperatures. This means HEATER_X_RAW_LO_TEMP
 * will probably be caught when the heating element first turns on during the
 * preheating process, which will trigger a min_temp_error as a safety measure
 * and force stop everything.
 * To circumvent this limitation, we allow for a preheat time (during which,
 * min_temp_error won't be triggered) and add a min_temp buffer to handle
 * aberrant readings.
 *
 * If you want to enable this feature for your hotend thermistor(s)
 * uncomment and set values > 0 in the constants below
 */

// The number of consecutive low temperature errors that can occur
// before a min_temp_error is triggered. (Shouldn't be more than 10.)
//#define MAX_CONSECUTIVE_LOW_TEMPERATURE_ERROR_ALLOWED 0

// The number of milliseconds a hotend will preheat before starting to check
// the temperature. This value should NOT be set to the time it takes the
// hot end to reach the target temperature, but the time it takes to reach
// the minimum temperature your thermistor can read. The lower the better/safer.
// This shouldn't need to be more than 30 seconds (30000)
//#define MILLISECONDS_PREHEAT_TIME 0

// @section extruder

// Extruder runout prevention.
// If the machine is idle and the temperature over MINTEMP
// then extrude some filament every couple of SECONDS.
//#define EXTRUDER_RUNOUT_PREVENT
#if ENABLED(EXTRUDER_RUNOUT_PREVENT)
    #define EXTRUDER_RUNOUT_MINTEMP 190
    #define EXTRUDER_RUNOUT_SECONDS 30
    #define EXTRUDER_RUNOUT_SPEED 1500 // (mm/m)
    #define EXTRUDER_RUNOUT_EXTRUDE 5 // (mm)
#endif

// @section temperature

// Calibration for AD595 / AD8495 sensor to adjust temperature measurements.
// The final temperature is calculated as (measuredTemp * GAIN) + OFFSET.
#define TEMP_SENSOR_AD595_OFFSET 0.0
#define TEMP_SENSOR_AD595_GAIN 1.0
#define TEMP_SENSOR_AD8495_OFFSET 0.0
#define TEMP_SENSOR_AD8495_GAIN 1.0

/**
 * Controller Fan
 * To cool down the stepper drivers and MOSFETs.
 *
 * The fan will turn on automatically whenever any stepper is enabled
 * and turn off after a set period after all steppers are turned off.
 */
//#define USE_CONTROLLER_FAN
#if ENABLED(USE_CONTROLLER_FAN)
    //#define CONTROLLER_FAN_PIN -1        // Set a custom pin for the controller fan
    #define CONTROLLERFAN_SECS 60 // Duration in seconds for the fan to run after all motors are disabled
    #define CONTROLLERFAN_SPEED 255 // 255 == full speed
#endif

// When first starting the main fan, run it at full speed for the
// given number of milliseconds.  This gets the fan spinning reliably
// before setting a PWM value. (Does not work with software PWM for fan on Sanguinololu)
//#define FAN_KICKSTART_TIME 100

/**
 * PWM Fan Scaling
 *
 * Define the min/max speeds for PWM fans (as set with M106).
 *
 * With these options the M106 0-255 value range is scaled to a subset
 * to ensure that the fan has enough power to spin, or to run lower
 * current fans with higher current. (e.g., 5V/12V fans with 12V/24V)
 * Value 0 always turns off the fan.
 *
 * Define one or both of these to override the default 0-255 range.
 */
//#define FAN_MIN_PWM 50
//#define FAN_MAX_PWM 128

/**
 * FAST PWM FAN Settings
 *
 * Use to change the FAST FAN PWM frequency (if enabled in Configuration.h)
 * Combinations of PWM Modes, prescale values and TOP resolutions are used internally to produce a
 * frequency as close as possible to the desired frequency.
 *
 * FAST_PWM_FAN_FREQUENCY [undefined by default]
 *   Set this to your desired frequency.
 *   If left undefined this defaults to F = F_CPU/(2*255*1)
 *   ie F = 31.4 Khz on 16 MHz microcontrollers or F = 39.2 KHz on 20 MHz microcontrollers
 *   These defaults are the same as with the old FAST_PWM_FAN implementation - no migration is required
 *   NOTE: Setting very low frequencies (< 10 Hz) may result in unexpected timer behaviour.
 *
 * USE_OCR2A_AS_TOP [undefined by default]
 *   Boards that use TIMER2 for PWM have limitations resulting in only a few possible frequencies on TIMER2:
 *   16MHz MCUs: [62.5KHz, 31.4KHz (default), 7.8KHz, 3.92KHz, 1.95KHz, 977Hz, 488Hz, 244Hz, 60Hz, 122Hz, 30Hz]
 *   20MHz MCUs: [78.1KHz, 39.2KHz (default), 9.77KHz, 4.9KHz, 2.44KHz, 1.22KHz, 610Hz, 305Hz, 153Hz, 76Hz, 38Hz]
 *   A greater range can be achieved by enabling USE_OCR2A_AS_TOP. But note that this option blocks the use of
 *   PWM on pin OC2A. Only use this option if you don't need PWM on 0C2A. (Check your schematic.)
 *   USE_OCR2A_AS_TOP sacrifices duty cycle control resolution to achieve this broader range of frequencies.
 */
#if ENABLED(FAST_PWM_FAN)
//#define FAST_PWM_FAN_FREQUENCY 31400
//#define USE_OCR2A_AS_TOP
#endif

// @section extruder

/**
 * Extruder cooling fans
 *
 * Extruder auto fans automatically turn on when their extruders'
 * temperatures go above EXTRUDER_AUTO_FAN_TEMPERATURE.
 *
 * Your board's pins file specifies the recommended pins. Override those here
 * or set to -1 to disable completely.
 *
 * Multiple extruders can be assigned to the same pin in which case
 * the fan will turn on when any selected extruder is above the threshold.
 */
#define E0_AUTO_FAN_PIN -1
#define E1_AUTO_FAN_PIN -1
#define E2_AUTO_FAN_PIN -1
#define E3_AUTO_FAN_PIN -1
#define E4_AUTO_FAN_PIN -1
#define E5_AUTO_FAN_PIN -1
#define CHAMBER_AUTO_FAN_PIN -1
#define EXTRUDER_AUTO_FAN_TEMPERATURE 50
#define EXTRUDER_AUTO_FAN_SPEED 255 // 255 == full speed

/**
 * Part-Cooling Fan Multiplexer
 *
 * This feature allows you to digitally multiplex the fan output.
 * The multiplexer is automatically switched at tool-change.
 * Set FANMUX[012]_PINs below for up to 2, 4, or 8 multiplexed fans.
 */
#define FANMUX0_PIN -1
#define FANMUX1_PIN -1
#define FANMUX2_PIN -1

/**
 * M355 Case Light on-off / brightness
 */
//#define CASE_LIGHT_ENABLE
#if ENABLED(CASE_LIGHT_ENABLE)
    //#define CASE_LIGHT_PIN 4                  // Override the default pin if needed
    #define INVERT_CASE_LIGHT false // Set true if Case Light is ON when pin is LOW
    #define CASE_LIGHT_DEFAULT_ON true // Set default power-up state on
    #define CASE_LIGHT_DEFAULT_BRIGHTNESS 105 // Set default power-up brightness (0-255, requires PWM pin)
    //#define MENU_ITEM_CASE_LIGHT              // Add a Case Light option to the LCD main menu
    //#define CASE_LIGHT_USE_NEOPIXEL           // Use Neopixel LED as case light, requires NEOPIXEL_LED.
    #if ENABLED(CASE_LIGHT_USE_NEOPIXEL)
        #define CASE_LIGHT_NEOPIXEL_COLOR \
            { 255, 255, 255, 255 } // { Red, Green, Blue, White }
    #endif
#endif

//===========================================================================
//============================ Mechanical Settings ==========================
//===========================================================================

// @section homing

// If you want endstops to stay on (by default) even when not homing
// enable this option. Override at any time with M120, M121.
//#define ENDSTOPS_ALWAYS_ON_DEFAULT

// @section extras

//#define Z_LATE_ENABLE // Enable Z the last moment. Needed if your Z driver overheats.

// Employ an external closed loop controller. Override pins here if needed.
//#define EXTERNAL_CLOSED_LOOP_CONTROLLER
#if ENABLED(EXTERNAL_CLOSED_LOOP_CONTROLLER)
//#define CLOSED_LOOP_ENABLE_PIN        -1
//#define CLOSED_LOOP_MOVE_COMPLETE_PIN -1
#endif

/**
 * Dual Steppers / Dual Endstops
 *
 * This section will allow you to use extra E drivers to drive a second motor for X, Y, or Z axes.
 *
 * For example, set X_DUAL_STEPPER_DRIVERS setting to use a second motor. If the motors need to
 * spin in opposite directions set INVERT_X2_VS_X_DIR. If the second motor needs its own endstop
 * set X_DUAL_ENDSTOPS. This can adjust for "racking." Use X2_USE_ENDSTOP to set the endstop plug
 * that should be used for the second endstop. Extra endstops will appear in the output of 'M119'.
 *
 * Use X_DUAL_ENDSTOP_ADJUSTMENT to adjust for mechanical imperfection. After homing both motors
 * this offset is applied to the X2 motor. To find the offset home the X axis, and measure the error
 * in X2. Dual endstop offsets can be set at runtime with 'M666 X<offset> Y<offset> Z<offset>'.
 */

//#define X_DUAL_STEPPER_DRIVERS
#if ENABLED(X_DUAL_STEPPER_DRIVERS)
    #define INVERT_X2_VS_X_DIR true // Set 'true' if X motors should rotate in opposite directions
    //#define X_DUAL_ENDSTOPS
    #if ENABLED(X_DUAL_ENDSTOPS)
        #define X2_USE_ENDSTOP _XMAX_
        #define X_DUAL_ENDSTOPS_ADJUSTMENT 0
    #endif
#endif

//#define Y_DUAL_STEPPER_DRIVERS
#if ENABLED(Y_DUAL_STEPPER_DRIVERS)
    #define INVERT_Y2_VS_Y_DIR true // Set 'true' if Y motors should rotate in opposite directions
    //#define Y_DUAL_ENDSTOPS
    #if ENABLED(Y_DUAL_ENDSTOPS)
        #define Y2_USE_ENDSTOP _YMAX_
        #define Y_DUAL_ENDSTOPS_ADJUSTMENT 0
    #endif
#endif

//#define Z_DUAL_STEPPER_DRIVERS
#if ENABLED(Z_DUAL_STEPPER_DRIVERS)
    //#define Z_DUAL_ENDSTOPS
    #if ENABLED(Z_DUAL_ENDSTOPS)
        #define Z2_USE_ENDSTOP _XMAX_
        #define Z_DUAL_ENDSTOPS_ADJUSTMENT 0
    #endif
#endif

//#define Z_TRIPLE_STEPPER_DRIVERS
#if ENABLED(Z_TRIPLE_STEPPER_DRIVERS)
    //#define Z_TRIPLE_ENDSTOPS
    #if ENABLED(Z_TRIPLE_ENDSTOPS)
        #define Z2_USE_ENDSTOP _XMAX_
        #define Z3_USE_ENDSTOP _YMAX_
        #define Z_TRIPLE_ENDSTOPS_ADJUSTMENT2 0
        #define Z_TRIPLE_ENDSTOPS_ADJUSTMENT3 0
    #endif
#endif

/**
 * Dual X Carriage
 *
 * This setup has two X carriages that can move independently, each with its own hotend.
 * The carriages can be used to print an object with two colors or materials, or in
 * "duplication mode" it can print two identical or X-mirrored objects simultaneously.
 * The inactive carriage is parked automatically to prevent oozing.
 * X1 is the left carriage, X2 the right. They park and home at opposite ends of the X axis.
 * By default the X2 stepper is assigned to the first unused E plug on the board.
 *
 * The following Dual X Carriage modes can be selected with M605 S<mode>:
 *
 *   0 : (FULL_CONTROL) The slicer has full control over both X-carriages and can achieve optimal travel
 *       results as long as it supports dual X-carriages. (M605 S0)
 *
 *   1 : (AUTO_PARK) The firmware automatically parks and unparks the X-carriages on tool-change so
 *       that additional slicer support is not required. (M605 S1)
 *
 *   2 : (DUPLICATION) The firmware moves the second X-carriage and extruder in synchronization with
 *       the first X-carriage and extruder, to print 2 copies of the same object at the same time.
 *       Set the constant X-offset and temperature differential with M605 S2 X[offs] R[deg] and
 *       follow with M605 S2 to initiate duplicated movement.
 *
 *   3 : (MIRRORED) Formbot/Vivedino-inspired mirrored mode in which the second extruder duplicates
 *       the movement of the first except the second extruder is reversed in the X axis.
 *       Set the initial X offset and temperature differential with M605 S2 X[offs] R[deg] and
 *       follow with M605 S3 to initiate mirrored movement.
 */
//#define DUAL_X_CARRIAGE
#if ENABLED(DUAL_X_CARRIAGE)
    #define X1_MIN_POS X_MIN_POS // Set to X_MIN_POS
    #define X1_MAX_POS X_BED_SIZE // Set a maximum so the first X-carriage can't hit the parked second X-carriage
    #define X2_MIN_POS 80 // Set a minimum to ensure the  second X-carriage can't hit the parked first X-carriage
    #define X2_MAX_POS 353 // Set this to the distance between toolheads when both heads are homed
    #define X2_HOME_DIR 1 // Set to 1. The second X-carriage always homes to the maximum endstop position
    #define X2_HOME_POS X2_MAX_POS // Default X2 home position. Set to X2_MAX_POS.
// However: In this mode the HOTEND_OFFSET_X value for the second extruder provides a software
// override for X2_HOME_POS. This also allow recalibration of the distance between the two endstops
// without modifying the firmware (through the "M218 T1 X???" command).
// Remember: you should set the second extruder x-offset to 0 in your slicer.

    // This is the default power-up mode which can be later using M605.
    #define DEFAULT_DUAL_X_CARRIAGE_MODE DXC_AUTO_PARK_MODE

    // Default x offset in duplication mode (typically set to half print bed width)
    #define DEFAULT_DUPLICATION_X_OFFSET 100

#endif // DUAL_X_CARRIAGE

// Activate a solenoid on the active extruder with M380. Disable all with M381.
// Define SOL0_PIN, SOL1_PIN, etc., for each extruder that has a solenoid.
//#define EXT_SOLENOID

// @section homing

//after enabling HOMING_MAX_ATTEMPTS, homing can fail
#define HOMING_MAX_ATTEMPTS 10

// Homing hits each endstop, retracts by these distances, then does a slower bump.
#define X_HOME_BUMP_MM 15
#define Y_HOME_BUMP_MM 20
#define Z_HOME_BUMP_MM 2
#define HOMING_BUMP_DIVISOR \
    { 1, 1, 4 } // Re-Bump Speed Divisor (Divides the Homing Feedrate)
#define HOMING_BUMP_DIVISOR_MAX HOMING_BUMP_DIVISOR
#define HOMING_BUMP_DIVISOR_MIN HOMING_BUMP_DIVISOR
//#define QUICK_HOME                     // If homing includes X and Y, do a diagonal move initially

// Move away from the endstops after homing
#define HOMING_BACKOFF_POST_MM { 2, 2, 0 }

// When G28 is called, this option will make Y home before X
// X must be homed before Y to avoid Nozzle Cleaner
#define HOME_Y_BEFORE_X false

// Enable this if X or Y can't home without homing the other axis first.
#define CODEPENDENT_XY_HOMING true
#include <option/has_nozzle_cleaner.h>
#if HAS_NOZZLE_CLEANER()
    #define AVOID_NOZZLE_CLEANER_Y_FIRST false
#endif


/**
 * Z Steppers Auto-Alignment
 * Add the G34 command to align multiple Z steppers using a bed probe.
 */
//#define Z_STEPPER_AUTO_ALIGN
#if ENABLED(Z_STEPPER_AUTO_ALIGN)
    // Define probe X and Y positions for Z1, Z2 [, Z3]
    #define Z_STEPPER_ALIGN_X \
        { 10, 150, 290 }
    #define Z_STEPPER_ALIGN_Y \
        { 290, 10, 290 }
    // Set number of iterations to align
    #define Z_STEPPER_ALIGN_ITERATIONS 3
    // Enable to restore leveling setup after operation
    #define RESTORE_LEVELING_AFTER_G34
    // Use the amplification factor to de-/increase correction step.
    // In case the stepper (spindle) position is further out than the test point
    // Use a value > 1. NOTE: This may cause instability
    #define Z_STEPPER_ALIGN_AMP 1.0
    // Stop criterion. If the accuracy is better than this stop iterating early
    #define Z_STEPPER_ALIGN_ACC 0.02
#endif

// @section machine

#define AXIS_RELATIVE_MODES \
    { false, false, false, false }

// Add a Duplicate option for well-separated conjoined nozzles
//#define MULTI_NOZZLE_DUPLICATION

// By default pololu step drivers require an active high signal. However, some high power drivers require an active low signal as step.
#define INVERT_X_STEP_PIN false
#define INVERT_Y_STEP_PIN false
#define INVERT_Z_STEP_PIN false
#define INVERT_E_STEP_PIN false

// Default stepper release if idle. Set to 0 to deactivate.
// Steppers will shut down DEFAULT_STEPPER_DEACTIVE_TIME seconds after the last move when DISABLE_INACTIVE_? is true.
// Time can be set by M18 and M84.
#define DEFAULT_STEPPER_DEACTIVE_TIME 120
#define DISABLE_INACTIVE_X true
#define DISABLE_INACTIVE_Y true
#define DISABLE_INACTIVE_Z true // set to false if the nozzle will fall down on your printed part when print has finished.
#define DISABLE_INACTIVE_E true

#define DEFAULT_MINIMUMFEEDRATE 0.0 // minimum feedrate
#define DEFAULT_MINTRAVELFEEDRATE 0.0

#define HOME_AFTER_DEACTIVATE  // Require rehoming after steppers are deactivated

// @section lcd

#define MANUAL_FEEDRATE \
    { 50 * 60, 50 * 60, 8 * 60, 4 * 60 } // Feedrates for manual moves along X, Y, Z, E from panel

#if ENABLED(ULTIPANEL)
    #define MANUAL_E_MOVES_RELATIVE // Show LCD extruder moves as relative rather than absolute positions
    #define ULTIPANEL_FEEDMULTIPLY // Comment to disable setting feedrate multiplier via encoder
#endif

// @section extras

// minimum time in microseconds that a movement needs to take if the buffer is emptied.
#define DEFAULT_MINSEGMENTTIME 20000

// If defined the movements slow down when the look ahead buffer is only half full
#define SLOWDOWN

// Frequency limit
// See nophead's blog for more info
// Not working O
//#define XY_FREQUENCY_LIMIT  15

// Minimum planner junction speed. Sets the default minimum speed the planner plans for at the end
// of the buffer and all stops. This should not be much greater than zero and should only be changed
// if unwanted behavior is observed on a user's machine when running at very slow speeds.
#define MINIMUM_PLANNER_SPEED 0.05 // (mm/s)

//
// Backlash Compensation
// Adds extra movement to axes on direction-changes to account for backlash.
//
//#define BACKLASH_COMPENSATION
#if ENABLED(BACKLASH_COMPENSATION)
    // Define values for backlash distance and correction.
    // If BACKLASH_GCODE is enabled these values are the defaults.
    #define BACKLASH_DISTANCE_MM \
        { 0, 0, 0 } // (mm)
    #define BACKLASH_CORRECTION 0.0 // 0.0 = no correction; 1.0 = full correction

// Set BACKLASH_SMOOTHING_MM to spread backlash correction over multiple segments
// to reduce print artifacts. (Enabling this is costly in memory and computation!)
//#define BACKLASH_SMOOTHING_MM 3 // (mm)

// Add runtime configuration and tuning of backlash values (M425)
//#define BACKLASH_GCODE

    #if ENABLED(BACKLASH_GCODE)
        // Measure the Z backlash when probing (G29) and set with "M425 Z"
        #define MEASURE_BACKLASH_WHEN_PROBING

        #if ENABLED(MEASURE_BACKLASH_WHEN_PROBING)
            // When measuring, the probe will move up to BACKLASH_MEASUREMENT_LIMIT
            // mm away from point of contact in BACKLASH_MEASUREMENT_RESOLUTION
            // increments while checking for the contact to be broken.
            #define BACKLASH_MEASUREMENT_LIMIT 0.5 // (mm)
            #define BACKLASH_MEASUREMENT_RESOLUTION 0.005 // (mm)
            #define BACKLASH_MEASUREMENT_FEEDRATE Z_PROBE_SPEED_SLOW // (mm/m)
        #endif
    #endif
#endif

/**
 * Automatic backlash, position and hotend offset calibration
 *
 * Enable G425 to run automatic calibration using an electrically-
 * conductive cube, bolt, or washer mounted on the bed.
 *
 * G425 uses the probe to touch the top and sides of the calibration object
 * on the bed and measures and/or correct positional offsets, axis backlash
 * and hotend offsets.
 *
 * Note: HOTEND_OFFSET and CALIBRATION_OBJECT_CENTER must be set to within
 *       ±5mm of true values for G425 to succeed.
 */
//#define CALIBRATION_GCODE
#if ENABLED(CALIBRATION_GCODE)

    #define CALIBRATION_MEASUREMENT_RESOLUTION 0.01 // mm

    #define CALIBRATION_FEEDRATE_SLOW 60 // mm/m
    #define CALIBRATION_FEEDRATE_FAST 1200 // mm/m
    #define CALIBRATION_FEEDRATE_TRAVEL 3000 // mm/m

    // The following parameters refer to the conical section of the nozzle tip.
    #define CALIBRATION_NOZZLE_TIP_HEIGHT 1.0 // mm
    #define CALIBRATION_NOZZLE_OUTER_DIAMETER 2.0 // mm

    // Uncomment to enable reporting (required for "G425 V", but consumes PROGMEM).
    //#define CALIBRATION_REPORTING

    // The true location and dimension the cube/bolt/washer on the bed.
    #define CALIBRATION_OBJECT_CENTER \
        { 264.0, -22.0, -2.0 } // mm
    #define CALIBRATION_OBJECT_DIMENSIONS \
        { 10.0, 10.0, 10.0 } // mm

    // Comment out any sides which are unreachable by the probe. For best
    // auto-calibration results, all sides must be reachable.
    #define CALIBRATION_MEASURE_RIGHT
    #define CALIBRATION_MEASURE_FRONT
    #define CALIBRATION_MEASURE_LEFT
    #define CALIBRATION_MEASURE_BACK

    // Probing at the exact top center only works if the center is flat. If
    // probing on a screwhead or hollow washer, probe near the edges.
    //#define CALIBRATION_MEASURE_AT_TOP_EDGES

    // Define pin which is read during calibration
    #ifndef CALIBRATION_PIN
        #define CALIBRATION_PIN -1 // Override in pins.h or set to -1 to use your Z endstop
        #define CALIBRATION_PIN_INVERTING false // set to true to invert the pin
        //#define CALIBRATION_PIN_PULLDOWN
        #define CALIBRATION_PIN_PULLUP
    #endif
#endif

/**
 * Adaptive Step Smoothing increases the resolution of multi-axis moves, particularly at step frequencies
 * below 1kHz (for AVR) or 10kHz (for ARM), where aliasing between axes in multi-axis moves causes audible
 * vibration and surface artifacts. The algorithm adapts to provide the best possible step smoothing at the
 * lowest stepping frequencies.
 */
//#define ADAPTIVE_STEP_SMOOTHING

/**
 * Custom Microstepping
 * Override as-needed for your setup. Up to 3 MS pins are supported.
 */
//#define MICROSTEP1 LOW,LOW,LOW
//#define MICROSTEP2 HIGH,LOW,LOW
//#define MICROSTEP4 LOW,HIGH,LOW
//#define MICROSTEP8 HIGH,HIGH,LOW
//#define MICROSTEP16 LOW,LOW,HIGH
//#define MICROSTEP32 HIGH,LOW,HIGH

// Microstep setting (Only functional when stepper driver microstep pins are connected to MCU.
//#define MICROSTEP_MODES { 16, 16, 16, 16, 16, 16 } // [1,2,4,8,16]

/**
 *  @section  stepper motor current
 *
 *  Some boards have a means of setting the stepper motor current via firmware.
 *
 *  The power on motor currents are set by:
 *    PWM_MOTOR_CURRENT - used by MINIRAMBO & ULTIMAIN_2
 *                         known compatible chips: A4982
 *    DIGIPOT_MOTOR_CURRENT - used by BQ_ZUM_MEGA_3D, RAMBO & SCOOVO_X9H
 *                         known compatible chips: AD5206
 *    DAC_MOTOR_CURRENT_DEFAULT - used by PRINTRBOARD_REVF & RIGIDBOARD_V2
 *                         known compatible chips: MCP4728
 *    DIGIPOT_I2C_MOTOR_CURRENTS - used by 5DPRINT, AZTEEG_X3_PRO, AZTEEG_X5_MINI_WIFI, MIGHTYBOARD_REVE
 *                         known compatible chips: MCP4451, MCP4018
 *
 *  Motor currents can also be set by M907 - M910 and by the LCD.
 *    M907 - applies to all.
 *    M908 - BQ_ZUM_MEGA_3D, RAMBO, PRINTRBOARD_REVF, RIGIDBOARD_V2 & SCOOVO_X9H
 *    M909, M910 & LCD - only PRINTRBOARD_REVF & RIGIDBOARD_V2
 */
//#define PWM_MOTOR_CURRENT { 1300, 1300, 1250 }          // Values in milliamps
//#define DIGIPOT_MOTOR_CURRENT { 135,135,135,135,135 }   // Values 0-255 (RAMBO 135 = ~0.75A, 185 = ~1A)
//#define DAC_MOTOR_CURRENT_DEFAULT { 70, 80, 90, 80 }    // Default drive percent - X, Y, Z, E axis

// Use an I2C based DIGIPOT (e.g., Azteeg X3 Pro)
//#define DIGIPOT_I2C
#if ENABLED(DIGIPOT_I2C) && !defined(DIGIPOT_I2C_ADDRESS_A)
    /**
   * Common slave addresses:
   *
   *                        A   (A shifted)   B   (B shifted)  IC
   * Smoothie              0x2C (0x58)       0x2D (0x5A)       MCP4451
   * AZTEEG_X3_PRO         0x2C (0x58)       0x2E (0x5C)       MCP4451
   * AZTEEG_X5_MINI_WIFI         0x58              0x5C        MCP4451
   * MIGHTYBOARD_REVE      0x2F (0x5E)                         MCP4018
   */
    #define DIGIPOT_I2C_ADDRESS_A 0x2C // unshifted slave address for first DIGIPOT
    #define DIGIPOT_I2C_ADDRESS_B 0x2D // unshifted slave address for second DIGIPOT
#endif

//#define DIGIPOT_MCP4018          // Requires library from https://github.com/stawel/SlowSoftI2CMaster
#define DIGIPOT_I2C_NUM_CHANNELS 8 // 5DPRINT: 4     AZTEEG_X3_PRO: 8     MKS SBASE: 5
// Actual motor currents in Amps. The number of entries must match DIGIPOT_I2C_NUM_CHANNELS.
// These correspond to the physical drivers, so be mindful if the order is changed.
#define DIGIPOT_I2C_MOTOR_CURRENTS \
    { 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 } //  AZTEEG_X3_PRO

//===========================================================================
//=============================Additional Features===========================
//===========================================================================

// @section lcd

// Change values more rapidly when the encoder is rotated faster
#define ENCODER_RATE_MULTIPLIER
#if ENABLED(ENCODER_RATE_MULTIPLIER)
    #define ENCODER_10X_STEPS_PER_SEC 30 // (steps/s) Encoder rate for 10x speed
    #define ENCODER_100X_STEPS_PER_SEC 80 // (steps/s) Encoder rate for 100x speed
#endif

// Play a beep when the feedrate is changed from the Status Screen
//#define BEEP_ON_FEEDRATE_CHANGE
#if ENABLED(BEEP_ON_FEEDRATE_CHANGE)
    #define FEEDRATE_CHANGE_BEEP_DURATION 10
    #define FEEDRATE_CHANGE_BEEP_FREQUENCY 440
#endif

// Include a page of printer information in the LCD Main Menu
//#define LCD_INFO_MENU

// Scroll a longer status message into view
//#define STATUS_MESSAGE_SCROLLING

// On the Info Screen, display XY with one decimal place when possible
//#define LCD_DECIMAL_SMALL_XY

// The timeout (in ms) to return to the status screen from sub-menus
//#define LCD_TIMEOUT_TO_STATUS 15000

// Add an 'M73' G-code to set the current percentage
//#define LCD_SET_PROGRESS_MANUALLY

#if HAS_CHARACTER_LCD && HAS_PRINT_PROGRESS
    //#define LCD_PROGRESS_BAR              // Show a progress bar on HD44780 LCDs for SD printing
    #if ENABLED(LCD_PROGRESS_BAR)
        #define PROGRESS_BAR_BAR_TIME 2000 // (ms) Amount of time to show the bar
        #define PROGRESS_BAR_MSG_TIME 3000 // (ms) Amount of time to show the status message
        #define PROGRESS_MSG_EXPIRE 0 // (ms) Amount of time to retain the status message (0=forever)
    //#define PROGRESS_MSG_ONCE           // Show the message for MSG_TIME then clear it
    //#define LCD_PROGRESS_BAR_TEST       // Add a menu item to test the progress bar
    #endif
#endif

/**
 * LED Control Menu
 * Enable this feature to add LED Control to the LCD menu
 */
//#define LED_CONTROL_MENU
#if ENABLED(LED_CONTROL_MENU)
    #define LED_COLOR_PRESETS // Enable the Preset Color menu option
    #if ENABLED(LED_COLOR_PRESETS)
        #define LED_USER_PRESET_RED 255 // User defined RED value
        #define LED_USER_PRESET_GREEN 128 // User defined GREEN value
        #define LED_USER_PRESET_BLUE 0 // User defined BLUE value
        #define LED_USER_PRESET_WHITE 255 // User defined WHITE value
        #define LED_USER_PRESET_BRIGHTNESS 255 // User defined intensity
    //#define LED_USER_PRESET_STARTUP       // Have the printer display the user preset color on startup
    #endif
#endif // LED_CONTROL_MENU

#if ENABLED(SDSUPPORT)

    // Some RAMPS and other boards don't detect when an SD card is inserted. You can work
    // around this by connecting a push button or single throw switch to the pin defined
    // as SD_DETECT_PIN in your board's pins definitions.
    // This setting should be disabled unless you are using a push button, pulling the pin to ground.
    // Note: This is always disabled for ULTIPANEL (except ELB_FULL_GRAPHIC_CONTROLLER).
    #define SD_DETECT_INVERTED

    #define SD_FINISHED_STEPPERRELEASE true // Disable steppers when SD Print is finished
    #define SD_FINISHED_RELEASECOMMAND "M84 X Y Z E" // You might want to keep the Z enabled so your bed stays in place.

    // Reverse SD sort to show "more recent" files first, according to the card's FAT.
    // Since the FAT gets out of order with usage, SDCARD_SORT_ALPHA is recommended.
    #define SDCARD_RATHERRECENTFIRST

    // Add an option in the menu to run all auto#.g files
    //#define MENU_ADDAUTOSTART

    /**
   * Continue after Power-Loss (Creality3D)
   *
   * Store the current state to the SD Card at the start of each layer
   * during SD printing. If the recovery file is found at boot time, present
   * an option on the LCD screen to continue the print from the last-known
   * point in the file.
   */
    //#define POWER_LOSS_RECOVERY
    #if ENABLED(POWER_LOSS_RECOVERY)
    //#define POWER_LOSS_PIN   44     // Pin to detect power loss
    //#define POWER_LOSS_STATE HIGH   // State of pin indicating power loss
    #endif

    /**
   * Sort SD file listings in alphabetical order.
   *
   * With this option enabled, items on SD cards will be sorted
   * by name for easier navigation.
   *
   * By default...
   *
   *  - Use the slowest -but safest- method for sorting.
   *  - Folders are sorted to the top.
   *  - The sort key is statically allocated.
   *  - No added G-code (M34) support.
   *  - 40 item sorting limit. (Items after the first 40 are unsorted.)
   *
   * SD sorting uses static allocation (as set by SDSORT_LIMIT), allowing the
   * compiler to calculate the worst-case usage and throw an error if the SRAM
   * limit is exceeded.
   *
   *  - SDSORT_USES_RAM provides faster sorting via a static directory buffer.
   *  - SDSORT_USES_STACK does the same, but uses a local stack-based buffer.
   *  - SDSORT_CACHE_NAMES will retain the sorted file listing in RAM. (Expensive!)
   *  - SDSORT_DYNAMIC_RAM only uses RAM when the SD menu is visible. (Use with caution!)
   */
    //#define SDCARD_SORT_ALPHA

    // SD Card Sorting options
    #if ENABLED(SDCARD_SORT_ALPHA)
        #define SDSORT_LIMIT 40 // Maximum number of sorted items (10-256). Costs 27 bytes each.
        #define FOLDER_SORTING -1 // -1=above  0=none  1=below
        #define SDSORT_GCODE false // Allow turning sorting on/off with LCD and M34 g-code.
        #define SDSORT_USES_RAM false // Pre-allocate a static array for faster pre-sorting.
        #define SDSORT_USES_STACK false // Prefer the stack for pre-sorting to give back some SRAM. (Negated by next 2 options.)
        #define SDSORT_CACHE_NAMES false // Keep sorted items in RAM longer for speedy performance. Most expensive option.
        #define SDSORT_DYNAMIC_RAM false // Use dynamic allocation (within SD menus). Least expensive option. Set SDSORT_LIMIT before use!
        /**
         * Maximum number of 13-byte VFAT entries to use for sorting.
         * Note: Only affects SCROLL_LONG_FILENAMES with SDSORT_CACHE_NAMES but not SDSORT_DYNAMIC_RAM.
         */
        #define SDSORT_CACHE_VFATS 2
    #endif

    // This allows hosts to request long names for files and folders with M33
    //#define LONG_FILENAME_HOST_SUPPORT

    // Enable this option to scroll long filenames in the SD card menu
    //#define SCROLL_LONG_FILENAMES

    /**
   * This option allows you to abort SD printing when any endstop is triggered.
   * This feature must be enabled with "M540 S1" or from the LCD menu.
   * To have any effect, endstops must be enabled during SD printing.
   */
    //#define ABORT_ON_ENDSTOP_HIT_FEATURE_ENABLED

    /**
   * This option makes it easier to print the same SD Card file again.
   * On print completion the LCD Menu will open with the file selected.
   * You can just click to start the print, or navigate elsewhere.
   */
    //#define SD_REPRINT_LAST_SELECTED_FILE

    /**
   * Auto-report SdCard status with M27 S<seconds>
   */
    //#define AUTO_REPORT_SD_STATUS

    /**
   * Support for USB thumb drives using an Arduino USB Host Shield or
   * equivalent MAX3421E breakout board. The USB thumb drive will appear
   * to Marlin as an SD card.
   *
   * The MAX3421E must be assigned the same pins as the SD card reader, with
   * the following pin mapping:
   *
   *    SCLK, MOSI, MISO --> SCLK, MOSI, MISO
   *    INT              --> SD_DETECT_PIN
   *    SS               --> SDSS
   */
    #define USB_FLASH_DRIVE_SUPPORT
    #if ENABLED(USB_FLASH_DRIVE_SUPPORT)
        #define USB_CS_PIN SDSS
        #define USB_INTR_PIN SD_DETECT_PIN
    #endif

    /**
   * When using a bootloader that supports SD-Firmware-Flashing,
   * add a menu item to activate SD-FW-Update on the next reboot.
   *
   * Requires ATMEGA2560 (Arduino Mega)
   *
   * Tested with this bootloader:
   *   https://github.com/FleetProbe/MicroBridge-Arduino-ATMega2560
   */
    //#define SD_FIRMWARE_UPDATE
    #if ENABLED(SD_FIRMWARE_UPDATE)
        #define SD_FIRMWARE_UPDATE_EEPROM_ADDR 0x1FF
        #define SD_FIRMWARE_UPDATE_ACTIVE_VALUE 0xF0
        #define SD_FIRMWARE_UPDATE_INACTIVE_VALUE 0xFF
    #endif

// Add an optimized binary file transfer mode, initiated with 'M28 B1'
//#define BINARY_FILE_TRANSFER

#endif // SDSUPPORT

/**
 * Additional options for Graphical Displays
 *
 * Use the optimizations here to improve printing performance,
 * which can be adversely affected by graphical display drawing,
 * especially when doing several short moves, and when printing
 * on DELTA and SCARA machines.
 *
 * Some of these options may result in the display lagging behind
 * controller events, as there is a trade-off between reliable
 * printing performance versus fast display updates.
 */
#if HAS_GRAPHICAL_LCD
    // Show SD percentage next to the progress bar
    //#define DOGM_SD_PERCENT

    // Enable to save many cycles by drawing a hollow frame on the Info Screen
    #define XYZ_HOLLOW_FRAME

    // Enable to save many cycles by drawing a hollow frame on Menu Screens
    #define MENU_HOLLOW_FRAME

    // A bigger font is available for edit items. Costs 3120 bytes of PROGMEM.
    // Western only. Not available for Cyrillic, Kana, Turkish, Greek, or Chinese.
    //#define USE_BIG_EDIT_FONT

    // A smaller font may be used on the Info Screen. Costs 2300 bytes of PROGMEM.
    // Western only. Not available for Cyrillic, Kana, Turkish, Greek, or Chinese.
    //#define USE_SMALL_INFOFONT

    // Enable this option and reduce the value to optimize screen updates.
    // The normal delay is 10µs. Use the lowest value that still gives a reliable display.
    //#define DOGM_SPI_DELAY_US 5

    // Swap the CW/CCW indicators in the graphics overlay
    //#define OVERLAY_GFX_REVERSE

    /**
   * ST7920-based LCDs can emulate a 16 x 4 character display using
   * the ST7920 character-generator for very fast screen updates.
   * Enable LIGHTWEIGHT_UI to use this special display mode.
   *
   * Since LIGHTWEIGHT_UI has limited space, the position and status
   * message occupy the same line. Set STATUS_EXPIRE_SECONDS to the
   * length of time to display the status message before clearing.
   *
   * Set STATUS_EXPIRE_SECONDS to zero to never clear the status.
   * This will prevent position updates from being displayed.
   */
    #if ENABLED(U8GLIB_ST7920)
        //#define LIGHTWEIGHT_UI
        #if ENABLED(LIGHTWEIGHT_UI)
            #define STATUS_EXPIRE_SECONDS 20
        #endif
    #endif

    /**
   * Status (Info) Screen customizations
   * These options may affect code size and screen render time.
   * Custom status screens can forcibly override these settings.
   */
    //#define STATUS_COMBINE_HEATERS    // Use combined heater images instead of separate ones
    //#define STATUS_HOTEND_NUMBERLESS  // Use plain hotend icons instead of numbered ones (with 2+ hotends)
    #define STATUS_HOTEND_INVERTED // Show solid nozzle bitmaps when heating (Requires STATUS_HOTEND_ANIM)
    #define STATUS_HOTEND_ANIM // Use a second bitmap to indicate hotend heating
    #define STATUS_BED_ANIM // Use a second bitmap to indicate bed heating
//#define STATUS_ALT_BED_BITMAP     // Use the alternative bed bitmap
//#define STATUS_ALT_FAN_BITMAP     // Use the alternative fan bitmap
//#define STATUS_FAN_FRAMES 3       // :[0,1,2,3,4] Number of fan animation frames
//#define STATUS_HEAT_PERCENT       // Show heating in a progress bar

#endif // HAS_GRAPHICAL_LCD

// @section safety

// The hardware watchdog should reset the microcontroller disabling all outputs,
// in case the firmware gets stuck and doesn't do temperature regulation.
#ifndef _DEBUG
    #define USE_WATCHDOG

    #if ENABLED(USE_WATCHDOG)
    // If you have a watchdog reboot in an ArduinoMega2560 then the device will hang forever, as a watchdog reset will leave the watchdog on.
    // The "WATCHDOG_RESET_MANUAL" goes around this by not using the hardware reset.
    //  However, THIS FEATURE IS UNSAFE!, as it will only work if interrupts are disabled. And the code could hang in an interrupt routine with interrupts disabled.
    //#define WATCHDOG_RESET_MANUAL
    #endif
#endif //_DEBUG

// @section lcd

/**
 * Babystepping enables movement of the axes by tiny increments without changing
 * the current position values. This feature is used primarily to adjust the Z
 * axis in the first layer of a print in real-time.
 *
 * Warning: Does not respect endstops!
 */
#define BABYSTEPPING
#if ENABLED(BABYSTEPPING)
    #define BABYSTEP_WITHOUT_HOMING
    //#define BABYSTEP_XY                     // Also enable X/Y Babystepping. Not supported on DELTA!
    #define BABYSTEP_INVERT_Z false // Change if Z babysteps should go the other way
    #define BABYSTEP_MULTIPLICATOR 1 // Babysteps are very small. Increase for faster motion.

    //#define DOUBLECLICK_FOR_Z_BABYSTEPPING  // Double-click on the Status Screen for Z Babystepping.
    #if ENABLED(DOUBLECLICK_FOR_Z_BABYSTEPPING)
        /**
         * Maximum interval between clicks, in milliseconds.
         * Note: Extra time may be added to mitigate controller latency.
         */
        #define DOUBLECLICK_MAX_INTERVAL 1250
        #define BABYSTEP_ALWAYS_AVAILABLE // Allow babystepping at all times (not just during movement).
        //#define MOVE_Z_WHEN_IDLE              // Jump to the move Z menu on doubleclick when printer is idle.
        #if ENABLED(MOVE_Z_WHEN_IDLE)
            #define MOVE_Z_IDLE_MULTIPLICATOR 1 // Multiply 1mm by this factor for the move step size.
        #endif
    #endif

    //#define BABYSTEP_ZPROBE_OFFSET          // Combine M851 Z and Babystepping
    #if ENABLED(BABYSTEP_ZPROBE_OFFSET)
    //#define BABYSTEP_HOTEND_Z_OFFSET      // For multiple hotends, babystep relative Z offsets
    //#define BABYSTEP_ZPROBE_GFX_OVERLAY   // Enable graphical overlay on Z-offset editor
    #endif
#endif

// @section extruder

/**
 * Linear Pressure Control v1.5
 *
 * Assumption: advance [steps] = k * (delta velocity [steps/s])
 * K=0 means advance disabled.
 *
 * NOTE: K values for LIN_ADVANCE 1.5 differ from earlier versions!
 *
 * Set K around 0.22 for 3mm PLA Direct Drive with ~6.5cm between the drive gear and heatbreak.
 * Larger K values will be needed for flexible filament and greater distances.
 * If this algorithm produces a higher speed offset than the extruder can handle (compared to E jerk)
 * print acceleration will be reduced during the affected moves to keep within the limit.
 *
 * See http://marlinfw.org/docs/features/lin_advance.html for full instructions.
 * Mention @Sebastianv650 on GitHub to alert the author of any issues.
 */
//#define LIN_ADVANCE
#if ENABLED(LIN_ADVANCE)
    #define LIN_ADVANCE_K 0 // Unit: mm compression per 1mm/s extruder speed
//#define LA_DEBUG          // If enabled, this will generate debug information output over USB.
#endif

// @section leveling

#if EITHER(MESH_BED_LEVELING, AUTO_BED_LEVELING_UBL)
// Override the mesh area if the automatic (max) area is too large
#define MESH_MIN_X ((-(X_BED_SIZE / (GRID_MAX_POINTS_X - 2 - 1))) + 15)
#define MESH_MIN_Y ((-(Y_BED_SIZE / (GRID_MAX_POINTS_Y - 2 - 1))) + 15)
#define MESH_MAX_X X_BED_SIZE - (MESH_MIN_X)
#define MESH_MAX_Y Y_BED_SIZE - (MESH_MIN_Y)
#endif

/**
 * Repeatedly attempt G29 leveling until it succeeds.
 * Stop after G29_MAX_RETRIES attempts.
 */
//#define G29_RETRY_AND_RECOVER
#if ENABLED(G29_RETRY_AND_RECOVER)
    #define G29_MAX_RETRIES 3
    #define G29_HALT_ON_FAILURE
    /**
   * Specify the GCODE commands that will be executed when leveling succeeds,
   * between attempts, and after the maximum number of retries have been tried.
   */
    #define G29_SUCCESS_COMMANDS "M117 Bed leveling done."
    #define G29_RECOVER_COMMANDS "M117 Probe failed. Rewiping.\nG28\nG12 P0 S12 T0"
    #define G29_FAILURE_COMMANDS "M117 Bed leveling failed.\nG0 Z10\nM300 P25 S880\nM300 P50 S0\nM300 P25 S880\nM300 P50 S0\nM300 P25 S880\nM300 P50 S0\nG4 S1"

#endif

// @section extras

//
// G2/G3 Arc Support
//
#define ARC_SUPPORT                      // Requires ~3226 bytes
#if ENABLED(ARC_SUPPORT)
  #define MIN_ARC_SEGMENT_MM        0.1  // (mm) Minimum length of each arc segment
  #define MAX_ARC_SEGMENT_MM        2.0  // (mm) Maximum length of each arc segment
  #define MIN_ARC_SEGMENTS_PER_SEC 50    // Use the feedrate to choose the segment length
  #define MAX_ARC_DEVIATION         0.02 // Maximum deviation from ideal arc due to segmentation
  #define N_ARC_CORRECTION         25    // Number of interpolated segments between corrections
  //#define ARC_P_CIRCLES                // Enable the 'P' parameter to specify complete circles
  //#define SF_ARC_FIX                   // Enable only if using SkeinForge with "Arc Point" fillet procedure
#endif

// G5 Bézier Curve Support with XYZE destination and IJPQ offsets
//#define BEZIER_CURVE_SUPPORT        // Requires ~2666 bytes

#if EITHER(ARC_SUPPORT, BEZIER_CURVE_SUPPORT)
  //#define CNC_WORKSPACE_PLANES      // Allow G2/G3/G5 to operate in XY, ZX, or YZ planes
#endif

/**
 * Direct Stepping
 *
 * Comparable to the method used by Klipper, G6 direct stepping significantly
 * reduces motion calculations, increases top printing speeds, and results in
 * less step aliasing by calculating all motions in advance.
 * Preparing your G-code: https://github.com/colinrgodsey/step-daemon
 */
//#define DIRECT_STEPPING

/**
 * G38 Probe Target
 *
 * This option adds G38.2 and G38.3 (probe towards target)
 * and optionally G38.4 and G38.5 (probe away from target).
 * Set MULTIPLE_PROBING for G38 to probe more than once.
 */
//#define G38_PROBE_TARGET
#if ENABLED(G38_PROBE_TARGET)
    //#define G38_PROBE_AWAY        // Include G38.4 and G38.5 to probe away from target
    #define G38_MINIMUM_MOVE 0.0275 // (mm) Minimum distance that will produce a move.
#endif

// Moves (or segments) with fewer steps than this will be joined with the next move
#define MIN_STEPS_PER_SEGMENT 6

/**
 * Minimum delay after setting the stepper DIR (in ns)
 *     0 : No delay (Expect at least 10µS since one Stepper ISR must transpire)
 *    20 : Minimum for TMC2xxx drivers
 *   200 : Minimum for A4988 drivers
 *   400 : Minimum for A5984 drivers
 *   500 : Minimum for LV8729 drivers (guess, no info in datasheet)
 *   650 : Minimum for DRV8825 drivers
 *  1500 : Minimum for TB6600 drivers (guess, no info in datasheet)
 * 15000 : Minimum for TB6560 drivers (guess, no info in datasheet)
 *
 * Override the default value based on the driver type set in Configuration.h.
 */
//#define MINIMUM_STEPPER_DIR_DELAY 650

/**
 * Minimum stepper driver pulse width (in µs)
 *   0 : Smallest possible width the MCU can produce, compatible with TMC2xxx drivers
 *   1 : Minimum for A4988, A5984, and LV8729 stepper drivers
 *   2 : Minimum for DRV8825 stepper drivers
 *   3 : Minimum for TB6600 stepper drivers
 *  30 : Minimum for TB6560 stepper drivers
 *
 * Override the default value based on the driver type set in Configuration.h.
 */
//#define MINIMUM_STEPPER_PULSE 2

/**
 * Maximum stepping rate (in Hz) the stepper driver allows
 *  If undefined, defaults to 1MHz / (2 * MINIMUM_STEPPER_PULSE)
 *  500000 : Maximum for A4988 stepper driver
 *  400000 : Maximum for TMC2xxx stepper drivers
 *  250000 : Maximum for DRV8825 stepper driver
 *  150000 : Maximum for TB6600 stepper driver
 *  130000 : Maximum for LV8729 stepper driver
 *   15000 : Maximum for TB6560 stepper driver
 *
 * Override the default value based on the driver type set in Configuration.h.
 */
//#define MAXIMUM_STEPPER_RATE 250000

// @section temperature

// Control heater 0 and heater 1 in parallel.
//#define HEATERS_PARALLEL

//===========================================================================
//================================= Buffers =================================
//===========================================================================

// @section hidden

// The number of linear motions that can be in the plan at any give time.
// THE BLOCK_BUFFER_SIZE NEEDS TO BE A POWER OF 2 (e.g. 8, 16, 32) because shifts and ors are used to do the ring-buffering.
#define BLOCK_BUFFER_SIZE 32

#define MOVE_SEGMENT_QUEUE_SIZE (2 * BLOCK_BUFFER_SIZE)
#define STEP_EVENT_QUEUE_SIZE   1024
#define ADVANCED_STEP_GENERATORS 1

// @section serial

// The ASCII buffer for serial input
#define MAX_CMD_SIZE 96
#define BUFSIZE 8

// Transmission to Host Buffer Size
// To save 386 bytes of PROGMEM (and TX_BUFFER_SIZE+3 bytes of RAM) set to 0.
// To buffer a simple "ok" you need 4 bytes.
// For ADVANCED_OK (M105) you need 32 bytes.
// For debug-echo: 128 bytes for the optimal speed.
// Other output doesn't need to be that speedy.
// :[0, 2, 4, 8, 16, 32, 64, 128, 256]
#define TX_BUFFER_SIZE 0

// Host Receive Buffer Size
// Without XON/XOFF flow control (see SERIAL_XON_XOFF below) 32 bytes should be enough.
// To use flow control, set this buffer size to at least 1024 bytes.
// :[0, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048]
//#define RX_BUFFER_SIZE 1024

#if RX_BUFFER_SIZE >= 1024
// Enable to have the controller send XON/XOFF control characters to
// the host to signal the RX buffer is becoming full.
//#define SERIAL_XON_XOFF
#endif

#if ENABLED(SDSUPPORT)
// Enable this option to collect and display the maximum
// RX queue usage after transferring a file to SD.
//#define SERIAL_STATS_MAX_RX_QUEUED

// Enable this option to collect and display the number
// of dropped bytes after a file transfer to SD.
//#define SERIAL_STATS_DROPPED_RX
#endif

// Enable an emergency-command parser to intercept certain commands as they
// enter the serial receive buffer, so they cannot be blocked.
// Currently handles M108, M112, M410
// Does not work on boards using AT90USB (USBCON) processors!
//#define EMERGENCY_PARSER

// Bad Serial-connections can miss a received command by sending an 'ok'
// Therefore some clients abort after 30 seconds in a timeout.
// Some other clients start sending commands while receiving a 'wait'.
// This "wait" is only sent when the buffer is empty. 1 second is a good value here.
//#define NO_TIMEOUTS 1000 // Milliseconds

// Some clients will have this feature soon. This could make the NO_TIMEOUTS unnecessary.
//#define ADVANCED_OK

// Printrun may have trouble receiving long strings all at once.
// This option inserts short delays between lines of serial output.
#define SERIAL_OVERRUN_PROTECTION

// @section extras

/**
 * Extra Fan Speed
 * Adds a secondary fan speed for each print-cooling fan.
 *   'M106 P<fan> T3-255' : Set a secondary speed for <fan>
 *   'M106 P<fan> T2'     : Use the set secondary speed
 *   'M106 P<fan> T1'     : Restore the previous fan speed
 */
//#define EXTRA_FAN_SPEED

/**
 * Firmware-based and LCD-controlled retract
 *
 * Add G10 / G11 commands for automatic firmware-based retract / recover.
 * Use M207 and M208 to define parameters for retract / recover.
 *
 * Use M209 to enable or disable auto-retract.
 * With auto-retract enabled, all G1 E moves within the set range
 * will be converted to firmware-based retract/recover moves.
 *
 * Be sure to turn off auto-retract during filament change.
 *
 * Note that M207 / M208 / M209 settings are saved to EEPROM.
 *
 */
//#define FWRETRACT
#if ENABLED(FWRETRACT)
    #define FWRETRACT_AUTORETRACT // costs ~500 bytes of PROGMEM
    #if ENABLED(FWRETRACT_AUTORETRACT)
        #define MIN_AUTORETRACT 0.1 // When auto-retract is on, convert E moves of this length and over
        #define MAX_AUTORETRACT 10.0 // Upper limit for auto-retract conversion
    #endif
    #define RETRACT_LENGTH 3 // Default retract length (positive mm)
    #define RETRACT_LENGTH_SWAP 13 // Default swap retract length (positive mm), for extruder change
    #define RETRACT_FEEDRATE 45 // Default feedrate for retracting (mm/s)
    #define RETRACT_ZRAISE 0 // Default retract Z-raise (mm)
    #define RETRACT_RECOVER_LENGTH 0 // Default additional recover length (mm, added to retract length when recovering)
    #define RETRACT_RECOVER_LENGTH_SWAP 0 // Default additional swap recover length (mm, added to retract length when recovering from extruder change)
    #define RETRACT_RECOVER_FEEDRATE 8 // Default feedrate for recovering from retraction (mm/s)
    #define RETRACT_RECOVER_FEEDRATE_SWAP 8 // Default feedrate for recovering from swap retraction (mm/s)
    #if ENABLED(MIXING_EXTRUDER)
    //#define RETRACT_SYNC_MIXING         // Retract and restore all mixing steppers simultaneously
    #endif
#endif

/**
 * Universal tool change settings.
 * Applies to all types of extruders except where explicitly noted.
 */
#if EXTRUDERS > 1
    // Z raise distance for tool-change, as needed for some extruders
    #define TOOLCHANGE_ZRAISE 2 // (mm)

    // Retract and prime filament on tool-change
    //#define TOOLCHANGE_FILAMENT_SWAP
    #if ENABLED(TOOLCHANGE_FILAMENT_SWAP)
        #define TOOLCHANGE_FIL_SWAP_LENGTH 12 // (mm)
        #define TOOLCHANGE_FIL_EXTRA_PRIME 2 // (mm)
        #define TOOLCHANGE_FIL_SWAP_RETRACT_SPEED 3600 // (mm/m)
        #define TOOLCHANGE_FIL_SWAP_PRIME_SPEED 3600 // (mm/m)
    #endif

    /**
   * Position to park head during tool change.
   * Doesn't apply to SWITCHING_TOOLHEAD, DUAL_X_CARRIAGE, or PARKING_EXTRUDER
   */
    //#define TOOLCHANGE_PARK
    #if ENABLED(TOOLCHANGE_PARK)
        #define TOOLCHANGE_PARK_XY \
            { X_MIN_POS + 10, Y_MIN_POS + 10 }
        #define TOOLCHANGE_PARK_XY_FEEDRATE 6000 // (mm/m)
    #endif
#endif

/**
 * Advanced Pause
 * Experimental feature for filament change support and for parking the nozzle when paused.
 * Adds the GCode M600 for initiating filament change.
 * If PARK_HEAD_ON_PAUSE enabled, adds the GCode M125 to pause printing and park the nozzle.
 *
 * Requires an LCD display.
 * Requires NOZZLE_PARK_FEATURE.
 * This feature is required for the default FILAMENT_RUNOUT_SCRIPT.
 */
#define ADVANCED_PAUSE_FEATURE
#if ENABLED(ADVANCED_PAUSE_FEATURE)
    #define FILAMENT_UNLOAD_RAMMING_SEQUENCE \
        { \
            { 20, 1200 }, \
            { 2, 2400 }, \
            { -72, 2400 }, \
            { 40, 700 }, \
            { -55, 400 }, \
            { 81, 4000},\
            { 3, 3000},\
            { -25, 6000},\
            { -20, 300},\
            { 20, 300},\
            { -60, 300}\
        }
    #define FILAMENT_RUNOUT_RAMMING_SEQUENCE FILAMENT_UNLOAD_RAMMING_SEQUENCE

    #define PAUSE_PARK_RETRACT_FEEDRATE 40 // (mm/s) Initial retract feedrate.
    /**
     * (mm) Initial retract.
     * This short retract is done immediately, before parking the nozzle.
     */
    #define PAUSE_PARK_RETRACT_LENGTH 2
    #define FILAMENT_CHANGE_UNLOAD_FEEDRATE 25 // (mm/s) Unload filament feedrate. This can be pretty fast.
    #define FILAMENT_CHANGE_UNLOAD_ACCEL 25 // (mm/s^2) Lower acceleration may allow a faster feedrate.
    /**
     * (mm) The length of filament for a complete unload.
     * For Bowden, the full length of the tube and nozzle.
     * For direct drive, the full length of the nozzle.
     * Set to 0 for manual unloading.
     */
    #define FILAMENT_CHANGE_UNLOAD_LENGTH 110
    #define FILAMENT_CHANGE_SLOW_LOAD_FEEDRATE 6 // (mm/s) Slow move when starting load.
    /**
     * (mm) Slow length, to allow time to insert material.
     * 0 to disable start loading and skip to fast load only
     */
    #define FILAMENT_CHANGE_SLOW_LOAD_LENGTH 40
    #define FILAMENT_CHANGE_FAST_LOAD_FEEDRATE 25 // (mm/s) Load filament feedrate. This can be pretty fast.
    #define FILAMENT_CHANGE_FAST_LOAD_ACCEL 25 // (mm/s^2) Lower acceleration may allow a faster feedrate.
    /**
     * (mm) Load length of filament, from extruder gear to nozzle.
     * For Bowden, the full length of the tube and nozzle.
     * For direct drive, the full length of the nozzle.
     */
    #define FILAMENT_CHANGE_FAST_LOAD_LENGTH 20
    //#define ADVANCED_PAUSE_CONTINUOUS_PURGE       // Purge continuously up to the purge length until interrupted.
    #define ADVANCED_PAUSE_PURGE_FEEDRATE 3 // (mm/s) Extrude feedrate (after loading). Should be slower than load feedrate.
    /**
     * (mm) Length to extrude after loading.
     * Set to 0 for manual extrusion.
     * Filament can be extruded repeatedly from the Filament Change menu
     * until extrusion is consistent, and to purge old filament.
     */
    #define ADVANCED_PAUSE_PURGE_LENGTH 40
    #define ADVANCED_PAUSE_RESUME_PRIME 0 // (mm) Extra distance to prime nozzle after returning from park.

// Filament Unload does a Retract, Delay, and Purge first:
    #define FILAMENT_UNLOAD_RETRACT_LENGTH 13 // (mm) Unload initial retract length.
    #define FILAMENT_UNLOAD_DELAY 5000 // (ms) Delay for the filament to cool after retract.
    #define FILAMENT_UNLOAD_PURGE_LENGTH 8 // (mm) An unretract is done, then this length is purged.
    #define FILAMENT_UNLOAD_PURGE_FEEDRATE 66 // (mm/s)
    #define FILAMENT_UNLOAD_PHASE1_LENGHT 20 // (mm)fast phase
    #define FILAMENT_UNLOAD_PHASE2_LENGHT 30 // (mm)slow phase

    #define PAUSE_PARK_NOZZLE_TIMEOUT 45 // (seconds) Time limit before the nozzle is turned off for safety.
    #define FILAMENT_CHANGE_ALERT_BEEPS 10 // Number of alert beeps to play when a response is needed.
    #define PAUSE_PARK_NO_STEPPER_TIMEOUT // Enable for XYZ steppers to stay powered on during filament change.

//#define PARK_HEAD_ON_PAUSE                    // Park the nozzle during pause and filament change.
//#define HOME_BEFORE_FILAMENT_CHANGE           // Ensure homing has been completed prior to parking for filament change

    #define FILAMENT_LOAD_UNLOAD_GCODES // Add M701/M702 Load/Unload G-codes, plus Load/Unload in the LCD Prepare menu.
//#define FILAMENT_UNLOAD_ALL_EXTRUDERS         // Allow M702 to unload all extruders above a minimum target temp (as set by M302)
#endif

// @section tmc

/**
 * TMC26X Stepper Driver options
 *
 * The TMC26XStepper library is required for this stepper driver.
 * https://github.com/trinamic/TMC26XStepper
 */
#if HAS_DRIVER(TMC26X)

    #if AXIS_DRIVER_TYPE_X(TMC26X)
        #define X_MAX_CURRENT 1000 // (mA)
        #define X_SENSE_RESISTOR 91 // (mOhms)
        #define X_MICROSTEPS 16 // Number of microsteps
    #endif

    #if AXIS_DRIVER_TYPE_X2(TMC26X)
        #define X2_MAX_CURRENT 1000
        #define X2_SENSE_RESISTOR 91
        #define X2_MICROSTEPS 16
    #endif

    #if AXIS_DRIVER_TYPE_Y(TMC26X)
        #define Y_MAX_CURRENT 1000
        #define Y_SENSE_RESISTOR 91
        #define Y_MICROSTEPS 16
    #endif

    #if AXIS_DRIVER_TYPE_Y2(TMC26X)
        #define Y2_MAX_CURRENT 1000
        #define Y2_SENSE_RESISTOR 91
        #define Y2_MICROSTEPS 16
    #endif

    #if AXIS_DRIVER_TYPE_Z(TMC26X)
        #define Z_MAX_CURRENT 1000
        #define Z_SENSE_RESISTOR 91
        #define Z_MICROSTEPS 16
    #endif

    #if AXIS_DRIVER_TYPE_Z2(TMC26X)
        #define Z2_MAX_CURRENT 1000
        #define Z2_SENSE_RESISTOR 91
        #define Z2_MICROSTEPS 16
    #endif

    #if AXIS_DRIVER_TYPE_Z3(TMC26X)
        #define Z3_MAX_CURRENT 1000
        #define Z3_SENSE_RESISTOR 91
        #define Z3_MICROSTEPS 16
    #endif

    #if AXIS_DRIVER_TYPE_E0(TMC26X)
        #define E0_MAX_CURRENT 1000
        #define E0_SENSE_RESISTOR 91
        #define E0_MICROSTEPS 16
    #endif

    #if AXIS_DRIVER_TYPE_E1(TMC26X)
        #define E1_MAX_CURRENT 1000
        #define E1_SENSE_RESISTOR 91
        #define E1_MICROSTEPS 16
    #endif

    #if AXIS_DRIVER_TYPE_E2(TMC26X)
        #define E2_MAX_CURRENT 1000
        #define E2_SENSE_RESISTOR 91
        #define E2_MICROSTEPS 16
    #endif

    #if AXIS_DRIVER_TYPE_E3(TMC26X)
        #define E3_MAX_CURRENT 1000
        #define E3_SENSE_RESISTOR 91
        #define E3_MICROSTEPS 16
    #endif

    #if AXIS_DRIVER_TYPE_E4(TMC26X)
        #define E4_MAX_CURRENT 1000
        #define E4_SENSE_RESISTOR 91
        #define E4_MICROSTEPS 16
    #endif

    #if AXIS_DRIVER_TYPE_E5(TMC26X)
        #define E5_MAX_CURRENT 1000
        #define E5_SENSE_RESISTOR 91
        #define E5_MICROSTEPS 16
    #endif

#endif // TMC26X

// @section tmc_smart

/**
 * To use TMC2130, TMC2160, TMC2660, TMC5130, TMC5160 stepper drivers in SPI mode
 * connect your SPI pins to the hardware SPI interface on your board and define
 * the required CS pins in your `pins_MYBOARD.h` file. (e.g., RAMPS 1.4 uses AUX3
 * pins `X_CS_PIN 53`, `Y_CS_PIN 49`, etc.).
 * You may also use software SPI if you wish to use general purpose IO pins.
 *
 * To use TMC2208 stepper UART-configurable stepper drivers connect #_SERIAL_TX_PIN
 * to the driver side PDN_UART pin with a 1K resistor.
 * To use the reading capabilities, also connect #_SERIAL_RX_PIN to PDN_UART without
 * a resistor.
 * The drivers can also be used with hardware serial.
 *
 * TMCStepper library is required to use TMC stepper drivers.
 * https://github.com/teemuatlut/TMCStepper
 */
#if HAS_TRINAMIC

    constexpr float HOLD_MULTIPLIER[4] = {1, 1, 1, 1};  // Scales down the holding current from run current
    #define INTERPOLATE true // Interpolate X/Y/Z_MICROSTEPS to 256

    #if AXIS_IS_TMC(X)
        // 200 step
        #define X_CURRENT 500 // (mA) RMS current. Multiply by 1.414 for peak current.
        #define X_MICROSTEPS 16 // 0..256
        #define X_RSENSE 0.22
        #define X_CHAIN_POS 0
    #endif

    #if AXIS_IS_TMC(X2)
        #define X2_CURRENT 800
        #define X2_MICROSTEPS 16
        #define X2_RSENSE 0.11
    #endif

    #if AXIS_IS_TMC(Y)
        // 200 step
        #define Y_CURRENT 500
        #define Y_MICROSTEPS 16
        #define Y_RSENSE 0.22
        #define Y_CHAIN_POS 0
    #endif

    #if AXIS_IS_TMC(Y2)
        #define Y2_CURRENT 800
        #define Y2_MICROSTEPS 16
        #define Y2_RSENSE 0.11
    #endif

    #if AXIS_IS_TMC(Z)
        #define Z_CURRENT 600
        #define Z_MICROSTEPS 16
        #define Z_RSENSE 0.22
        #define Z_CHAIN_POS 0
    #endif

    #if AXIS_IS_TMC(Z2)
        #define Z2_CURRENT 800
        #define Z2_MICROSTEPS 16
        #define Z2_RSENSE 0.11
    #endif

    #if AXIS_IS_TMC(Z3)
        #define Z3_CURRENT 800
        #define Z3_MICROSTEPS 16
        #define Z3_RSENSE 0.11
    #endif

    #if AXIS_IS_TMC(E0)
        #define E0_CURRENT 450
        #define E0_MICROSTEPS 16
        #define E0_RSENSE 0.22
        #define E0_CHAIN_POS 0
    #endif

    #if AXIS_IS_TMC(E1)
        #define E1_CURRENT 800
        #define E1_MICROSTEPS 16
        #define E1_RSENSE 0.11
    #endif

    #if AXIS_IS_TMC(E2)
        #define E2_CURRENT 800
        #define E2_MICROSTEPS 16
        #define E2_RSENSE 0.11
    #endif

    #if AXIS_IS_TMC(E3)
        #define E3_CURRENT 800
        #define E3_MICROSTEPS 16
        #define E3_RSENSE 0.11
    #endif

    #if AXIS_IS_TMC(E4)
        #define E4_CURRENT 800
        #define E4_MICROSTEPS 16
        #define E4_RSENSE 0.11
    #endif

    #if AXIS_IS_TMC(E5)
        #define E5_CURRENT 800
        #define E5_MICROSTEPS 16
        #define E5_RSENSE 0.11
    #endif

    /**
   * Override default SPI pins for TMC2130, TMC2160, TMC2660, TMC5130 and TMC5160 drivers here.
   * The default pins can be found in your board's pins file.
   */
    //#define X_CS_PIN          -1
    //#define Y_CS_PIN          -1
    //#define Z_CS_PIN          -1
    //#define X2_CS_PIN         -1
    //#define Y2_CS_PIN         -1
    //#define Z2_CS_PIN         -1
    //#define Z3_CS_PIN         -1
    //#define E0_CS_PIN         -1
    //#define E1_CS_PIN         -1
    //#define E2_CS_PIN         -1
    //#define E3_CS_PIN         -1
    //#define E4_CS_PIN         -1
    //#define E5_CS_PIN         -1

    /**
   * Use software SPI for TMC2130.
   * Software option for SPI driven drivers (TMC2130, TMC2160, TMC2660, TMC5130 and TMC5160).
   * The default SW SPI pins are defined the respective pins files,
   * but you can override or define them here.
   */
    //#define TMC_USE_SW_SPI
    //#define TMC_SW_MOSI       -1
    //#define TMC_SW_MISO       -1
    //#define TMC_SW_SCK        -1

    /**
   * Software enable
   *
   * Use for drivers that do not use a dedicated enable pin, but rather handle the same
   * function through a communication line such as SPI or UART.
   */
    //#define SOFTWARE_DRIVER_ENABLE

    /**
   * TMC2130, TMC2160, TMC2208, TMC5130 and TMC5160 only
   * Use Trinamic's ultra quiet stepping mode.
   * When disabled, Marlin will use spreadCycle stepping mode.
   */
    //#define STEALTHCHOP_XY
    //#define STEALTHCHOP_Z
    //#define STEALTHCHOP_E

    /**
   * Optimize spreadCycle chopper parameters by using predefined parameter sets
   * or with the help of an example included in the library.
   * Provided parameter sets are
   * CHOPPER_DEFAULT_12V
   * CHOPPER_DEFAULT_19V
   * CHOPPER_DEFAULT_24V
   * CHOPPER_DEFAULT_36V
   * CHOPPER_PRUSAMK3_24V // Imported parameters from the official Prusa firmware for MK3 (24V)
   * CHOPPER_MARLIN_119   // Old defaults from Marlin v1.1.9
   *
   * Define you own with
   * { <off_time[1..15]>, <hysteresis_end[-3..12]>, hysteresis_start[1..8] }
   */
    #define CHOPPER_TIMING CHOPPER_PRUSAMK3_24V

/**
   * Monitor Trinamic drivers for error conditions,
   * like overtemperature and short to ground. TMC2208 requires hardware serial.
   * In the case of overtemperature Marlin can decrease the driver current until error condition clears.
   * Other detected conditions can be used to stop the current print.
   * Relevant g-codes:
   * M906 - Set or get motor current in milliamps using axis codes X, Y, Z, E. Report values if no axis codes given.
   * M911 - Report stepper driver overtemperature pre-warn condition.
   * M912 - Clear stepper driver overtemperature pre-warn condition flag.
   * M122 - Report driver parameters (Requires TMC_DEBUG)
   */
//#define MONITOR_DRIVER_STATUS

    #if ENABLED(MONITOR_DRIVER_STATUS)
        #define CURRENT_STEP_DOWN 50 // [mA]
        #define REPORT_CURRENT_CHANGE
        #define STOP_ON_ERROR
    #endif

/**
   * TMC2130, TMC2160, TMC2208, TMC5130 and TMC5160 only
   * The driver will switch to spreadCycle when stepper speed is over HYBRID_THRESHOLD.
   * This mode allows for faster movements at the expense of higher noise levels.
   * STEALTHCHOP_(XY|Z|E) must be enabled to use HYBRID_THRESHOLD.
   * M913 X/Y/Z/E to live tune the setting
   */
//#define HYBRID_THRESHOLD

    #define X_HYBRID_THRESHOLD 100 // [mm/s]
    #define X2_HYBRID_THRESHOLD 100
    #define Y_HYBRID_THRESHOLD 100
    #define Y2_HYBRID_THRESHOLD 100
    #define Z_HYBRID_THRESHOLD 3
    #define Z2_HYBRID_THRESHOLD 3
    #define Z3_HYBRID_THRESHOLD 3
    #define E0_HYBRID_THRESHOLD 30
    #define E1_HYBRID_THRESHOLD 30
    #define E2_HYBRID_THRESHOLD 30
    #define E3_HYBRID_THRESHOLD 30
    #define E4_HYBRID_THRESHOLD 30
    #define E5_HYBRID_THRESHOLD 30

/**
 * Provides crash detection during printing and proper crash recovery.
 * Sensorless homing must be turned on and sensitivities set accordingly.
 */
#define CRASH_RECOVERY
#ifdef CRASH_RECOVERY
    #define CRASH_STALL_GUARD { 2, 2 } // internal value representing sensitivity
    #define CRASH_MAX_PERIOD { STALL_THRESHOLD_TMC2130, STALL_THRESHOLD_TMC2130 }
    #define CRASH_FILTER (false)       // Stallguard filtering for crash detection
    #define CRASH_TIMER 45             // seconds before counter reset
    #define CRASH_COUNTER_MAX 3        // max crashes with automatic recovery
#endif

/**
 * Recovery from power failure. This is a distinct implementation from
 * POWER_LOSS_RECOVERY specific to Prusa printers.
 */
#define POWER_PANIC

#ifdef POWER_PANIC
    #define POWER_PANIC_Z_LIFT_CYCLES 4 // 4xFullStep cycles = ~0.64mm
    #define POWER_PANIC_MAX_BED_DIFF 10 // Maximum bed temperature (C) difference for auto-recovery

    // milliseconds to wait on hold before auto-restarting during short power failures
    #define POWER_PANIC_HOLD_RST_MS 5000

    #define POWER_PANIC_X_CURRENT 350 // (mA) RMS current for parking
    #define POWER_PANIC_X_FEEDRATE 200 // (mm/s, running at POWER_PANIC_X_CURRENT)

    #define POWER_PANIC_Z_CURRENT 350 // (mA) RMS current _after_ alignment
    #define POWER_PANIC_Z_FEEDRATE 50 // (mm/s, running at default current)

    #define POWER_PANIC_E_CURRENT 300 // (mA) RMS current
#endif

#define REBOOT_RESTORE_Z

/**
   * TMC2130, TMC2160, TMC2660, TMC5130, and TMC5160 only
   * Use StallGuard2 to sense an obstacle and trigger an endstop.
   * Connect the stepper driver's DIAG1 pin to the X/Y endstop pin.
   * X, Y, and Z homing will always be done in spreadCycle mode.
   *
   * X/Y/Z_STALL_SENSITIVITY is used for tuning the trigger sensitivity.
   * Higher values make the system LESS sensitive.
   * Lower value make the system MORE sensitive.
   * Too low values can lead to false positives, while too high values will collide the axis without triggering.
   * It is advised to set X/Y/Z_HOME_BUMP_MM to 0.
   * M914 X/Y/Z to live tune the setting
   *
   * Stall threshold defines maximal period between steps to trigger a stallguard
   */
#define SENSORLESS_HOMING

/**
   * Use StallGuard2 to probe the bed with the nozzle.
   *
   * CAUTION: This could cause damage to machines that use a lead screw or threaded rod
   *          to move the Z axis. Take extreme care when attempting to enable this feature.
   */
//#define SENSORLESS_PROBING

    #if EITHER(SENSORLESS_HOMING, SENSORLESS_PROBING)
        #define X_STALL_SENSITIVITY  0
        #define Y_STALL_SENSITIVITY  0
        #define Z_STALL_SENSITIVITY  4

        // TODO: this is just a ballpark value
        #define STALL_THRESHOLD_TMC2130 400 // (steps per tick) - reciprocal value of minimal speed
    #endif

    #ifdef IMPROVE_HOMING_RELIABILITY
        #define XY_HOMING_ACCELERATION 1250
        #define XY_HOMING_JERK 8
        #define X_CURRENT_HOME X_CURRENT
        #define Y_CURRENT_HOME Y_CURRENT
        #if ENABLED(PRECISE_HOMING_COREXY)
            #define PRECISE_HOMING_COREXY_RETRIES 3    // count: maximum number of refinement attempts
            #define XY_HOMING_HOLDING_CURRENT 900      // mA: holding current for fixed motor
            #define XY_HOMING_ORIGIN_OFFSET 5.f        // mm: parallel distance from initial origin
            #define XY_HOMING_ORIGIN_BUMP_RETRIES 6    // count: maximum number of measurement attempts
            #define XY_HOMING_ORIGIN_BUMP_MAX_ERR 0.15 // mm: max error between acceptable probes
        #endif
    #endif

    // Create a 50/50 square wave step pulse optimal for stepper drivers.
    #define SQUARE_WAVE_STEPPING

    /**
   * Enable M122 debugging command for TMC stepper drivers.
   * M122 S0/1 will enable continous reporting.
   */
    //#define TMC_DEBUG

    /**
   * You can set your own advanced settings by filling in predefined functions.
   * A list of available functions can be found on the library github page
   * https://github.com/teemuatlut/TMC2130Stepper
   * https://github.com/teemuatlut/TMC2208Stepper
   *
   * Example:
   * #define TMC_ADV() { \
   *   stepperX.diag0_temp_prewarn(1); \
   *   stepperY.interpolate(0); \
   * }
   */
    #define TMC_ADV() \
        {}

#endif // HAS_TRINAMIC

// @section L6470

/**
 * L6470 Stepper Driver options
 *
 * Arduino-L6470 library (0.7.0 or higher) is required for this stepper driver.
 * https://github.com/ameyer/Arduino-L6470
 *
 * Requires the following to be defined in your pins_YOUR_BOARD file
 *     L6470_CHAIN_SCK_PIN
 *     L6470_CHAIN_MISO_PIN
 *     L6470_CHAIN_MOSI_PIN
 *     L6470_CHAIN_SS_PIN
 *     L6470_RESET_CHAIN_PIN  (optional)
 */
#if HAS_DRIVER(L6470)

//#define L6470_CHITCHAT        // Display additional status info

    #if AXIS_DRIVER_TYPE_X(L6470)
        #define X_MICROSTEPS 128 // Number of microsteps (VALID: 1, 2, 4, 8, 16, 32, 128)
        #define X_OVERCURRENT 2000 // (mA) Current where the driver detects an over current (VALID: 375 x (1 - 16) - 6A max - rounds down)
        #define X_STALLCURRENT 1500 // (mA) Current where the driver detects a stall (VALID: 31.25 * (1-128) -  4A max - rounds down)
        #define X_MAX_VOLTAGE 127 // 0-255, Maximum effective voltage seen by stepper
        #define X_CHAIN_POS 0 // Position in SPI chain, 0=Not in chain, 1=Nearest MOSI
    #endif

    #if AXIS_DRIVER_TYPE_X2(L6470)
        #define X2_MICROSTEPS 128
        #define X2_OVERCURRENT 2000
        #define X2_STALLCURRENT 1500
        #define X2_MAX_VOLTAGE 127
        #define X2_CHAIN_POS 0
    #endif

    #if AXIS_DRIVER_TYPE_Y(L6470)
        #define Y_MICROSTEPS 128
        #define Y_OVERCURRENT 2000
        #define Y_STALLCURRENT 1500
        #define Y_MAX_VOLTAGE 127
        #define Y_CHAIN_POS 0
    #endif

    #if AXIS_DRIVER_TYPE_Y2(L6470)
        #define Y2_MICROSTEPS 128
        #define Y2_OVERCURRENT 2000
        #define Y2_STALLCURRENT 1500
        #define Y2_MAX_VOLTAGE 127
        #define Y2_CHAIN_POS 0
    #endif

    #if AXIS_DRIVER_TYPE_Z(L6470)
        #define Z_MICROSTEPS 128
        #define Z_OVERCURRENT 2000
        #define Z_STALLCURRENT 1500
        #define Z_MAX_VOLTAGE 127
        #define Z_CHAIN_POS 0
    #endif

    #if AXIS_DRIVER_TYPE_Z2(L6470)
        #define Z2_MICROSTEPS 128
        #define Z2_OVERCURRENT 2000
        #define Z2_STALLCURRENT 1500
        #define Z2_MAX_VOLTAGE 127
        #define Z2_CHAIN_POS 0
    #endif

    #if AXIS_DRIVER_TYPE_Z3(L6470)
        #define Z3_MICROSTEPS 128
        #define Z3_OVERCURRENT 2000
        #define Z3_STALLCURRENT 1500
        #define Z3_MAX_VOLTAGE 127
        #define Z3_CHAIN_POS 0
    #endif

    #if AXIS_DRIVER_TYPE_E0(L6470)
        #define E0_MICROSTEPS 128
        #define E0_OVERCURRENT 2000
        #define E0_STALLCURRENT 1500
        #define E0_MAX_VOLTAGE 127
        #define E0_CHAIN_POS 0
    #endif

    #if AXIS_DRIVER_TYPE_E1(L6470)
        #define E1_MICROSTEPS 128
        #define E1_OVERCURRENT 2000
        #define E1_STALLCURRENT 1500
        #define E1_MAX_VOLTAGE 127
        #define E1_CHAIN_POS 0
    #endif

    #if AXIS_DRIVER_TYPE_E2(L6470)
        #define E2_MICROSTEPS 128
        #define E2_OVERCURRENT 2000
        #define E2_STALLCURRENT 1500
        #define E2_MAX_VOLTAGE 127
        #define E2_CHAIN_POS 0
    #endif

    #if AXIS_DRIVER_TYPE_E3(L6470)
        #define E3_MICROSTEPS 128
        #define E3_OVERCURRENT 2000
        #define E3_STALLCURRENT 1500
        #define E3_MAX_VOLTAGE 127
        #define E3_CHAIN_POS 0
    #endif

    #if AXIS_DRIVER_TYPE_E4(L6470)
        #define E4_MICROSTEPS 128
        #define E4_OVERCURRENT 2000
        #define E4_STALLCURRENT 1500
        #define E4_MAX_VOLTAGE 127
        #define E4_CHAIN_POS 0
    #endif

    #if AXIS_DRIVER_TYPE_E5(L6470)
        #define E5_MICROSTEPS 128
        #define E5_OVERCURRENT 2000
        #define E5_STALLCURRENT 1500
        #define E5_MAX_VOLTAGE 127
        #define E5_CHAIN_POS 0
    #endif

/**
   * Monitor L6470 drivers for error conditions like over temperature and over current.
   * In the case of over temperature Marlin can decrease the drive until the error condition clears.
   * Other detected conditions can be used to stop the current print.
   * Relevant g-codes:
   * M906 - I1/2/3/4/5  Set or get motor drive level using axis codes X, Y, Z, E. Report values if no axis codes given.
   *         I not present or I0 or I1 - X, Y, Z or E0
   *         I2 - X2, Y2, Z2 or E1
   *         I3 - Z3 or E3
   *         I4 - E4
   *         I5 - E5
   * M916 - Increase drive level until get thermal warning
   * M917 - Find minimum current thresholds
   * M918 - Increase speed until max or error
   * M122 S0/1 - Report driver parameters
   */
//#define MONITOR_L6470_DRIVER_STATUS

    #if ENABLED(MONITOR_L6470_DRIVER_STATUS)
        #define KVAL_HOLD_STEP_DOWN 1
    //#define L6470_STOP_ON_ERROR
    #endif

#endif // L6470

// @section extras

/**
 * Photo G-code
 * Add the M240 G-code to take a photo.
 * The photo can be triggered by a digital pin or a physical movement.
 */
//#define PHOTO_GCODE
#if ENABLED(PHOTO_GCODE)
// A position to move to (and raise Z) before taking the photo
//#define PHOTO_POSITION { X_MAX_POS - 5, Y_MAX_POS, 0 }  // { xpos, ypos, zraise } (M240 X Y Z)
//#define PHOTO_DELAY_MS   100                            // (ms) Duration to pause before moving back (M240 P)
//#define PHOTO_RETRACT_MM   6.5                          // (mm) E retract/recover for the photo move (M240 R S)

// Canon RC-1 or homebrew digital camera trigger
// Data from: http://www.doc-diy.net/photo/rc-1_hacked/
//#define PHOTOGRAPH_PIN 23

// Canon Hack Development Kit
// http://captain-slow.dk/2014/03/09/3d-printing-timelapses/
//#define CHDK_PIN        4

// Optional second move with delay to trigger the camera shutter
//#define PHOTO_SWITCH_POSITION { X_MAX_POS, Y_MAX_POS }  // { xpos, ypos } (M240 I J)

// Duration to hold the switch or keep CHDK_PIN high
//#define PHOTO_SWITCH_MS   50 // (ms) (M240 D)
#endif

/**
 * Spindle & Laser control
 *
 * Add the M3, M4, and M5 commands to turn the spindle/laser on and off, and
 * to set spindle speed, spindle direction, and laser power.
 *
 * SuperPid is a router/spindle speed controller used in the CNC milling community.
 * Marlin can be used to turn the spindle on and off. It can also be used to set
 * the spindle speed from 5,000 to 30,000 RPM.
 *
 * You'll need to select a pin for the ON/OFF function and optionally choose a 0-5V
 * hardware PWM pin for the speed control and a pin for the rotation direction.
 *
 * See http://marlinfw.org/docs/configuration/laser_spindle.html for more config details.
 */
//#define SPINDLE_LASER_ENABLE
#if ENABLED(SPINDLE_LASER_ENABLE)

    #define SPINDLE_LASER_ENABLE_INVERT false // set to "true" if the on/off function is reversed
    #define SPINDLE_LASER_PWM true // set to true if your controller supports setting the speed/power
    #define SPINDLE_LASER_PWM_INVERT true // set to "true" if the speed/power goes up when you want it to go slower
    #define SPINDLE_LASER_POWERUP_DELAY 5000 // delay in milliseconds to allow the spindle/laser to come up to speed/power
    #define SPINDLE_LASER_POWERDOWN_DELAY 5000 // delay in milliseconds to allow the spindle to stop
    #define SPINDLE_DIR_CHANGE true // set to true if your spindle controller supports changing spindle direction
    #define SPINDLE_INVERT_DIR false
    #define SPINDLE_STOP_ON_DIR_CHANGE true // set to true if Marlin should stop the spindle before changing rotation direction

/**
   *  The M3 & M4 commands use the following equation to convert PWM duty cycle to speed/power
   *
   *  SPEED/POWER = PWM duty cycle * SPEED_POWER_SLOPE + SPEED_POWER_INTERCEPT
   *    where PWM duty cycle varies from 0 to 255
   *
   *  set the following for your controller (ALL MUST BE SET)
   */

    #define SPEED_POWER_SLOPE 118.4
    #define SPEED_POWER_INTERCEPT 0
    #define SPEED_POWER_MIN 5000
    #define SPEED_POWER_MAX 30000 // SuperPID router controller 0 - 30,000 RPM

//#define SPEED_POWER_SLOPE      0.3922
//#define SPEED_POWER_INTERCEPT  0
//#define SPEED_POWER_MIN       10
//#define SPEED_POWER_MAX      100      // 0-100%
#endif

/**
 * Filament Width Sensor
 *
 * Measures the filament width in real-time and adjusts
 * flow rate to compensate for any irregularities.
 *
 * Also allows the measured filament diameter to set the
 * extrusion rate, so the slicer only has to specify the
 * volume.
 *
 * Only a single extruder is supported at this time.
 *
 *  34 RAMPS_14    : Analog input 5 on the AUX2 connector
 *  81 PRINTRBOARD : Analog input 2 on the Exp1 connector (version B,C,D,E)
 * 301 RAMBO       : Analog input 3
 *
 * Note: May require analog pins to be defined for other boards.
 */
//#define FILAMENT_WIDTH_SENSOR

#if ENABLED(FILAMENT_WIDTH_SENSOR)
    #define FILAMENT_SENSOR_EXTRUDER_NUM 0 // Index of the extruder that has the filament sensor. :[0,1,2,3,4]
    #define MEASUREMENT_DELAY_CM 14 // (cm) The distance from the filament sensor to the melting chamber

    #define FILWIDTH_ERROR_MARGIN 1.0 // (mm) If a measurement differs too much from nominal width ignore it
    #define MAX_MEASUREMENT_DELAY 20 // (bytes) Buffer size for stored measurements (1 byte per cm). Must be larger than MEASUREMENT_DELAY_CM.

    #define DEFAULT_MEASURED_FILAMENT_DIA DEFAULT_NOMINAL_FILAMENT_DIA // Set measured to nominal initially

// Display filament width on the LCD status line. Status messages will expire after 5 seconds.
//#define FILAMENT_LCD_DISPLAY
#endif

/**
 * CNC Coordinate Systems
 *
 * Enables G53 and G54-G59.3 commands to select coordinate systems
 * and G92.1 to reset the workspace to native machine space.
 */
#define CNC_COORDINATE_SYSTEMS



/**
 * Z Axis height calibration.
 *
 *
 */

#define Z_AXIS_CALIBRATION //Enable G64 - measure Z axis size and save it to eeprom

#ifdef Z_AXIS_CALIBRATION
    #ifdef CNC_COORDINATE_SYSTEMS
        #define Z_SHIFTED_COOR_SYS 0    //Load Z axis size from eeprom to CNC_COORDINATE_SYSTEMS 0=G54 ... 9=G59.3
        #define MIN_SAVE_DIFFERENCE 0.5 //Don't save new Z_HEIGHT if difference between new and old Z_HEIGHT is lower than MIN_SAVE_DIFFERENCE. Used to extend EEPROM lifetime.
    #else
        #error This feature is useless without CNC_COORDINATE_SYSTEMS. Better disable it completly.
    #endif

#endif

/**
 * Auto-report temperatures with M155 S<seconds>
 */
#define AUTO_REPORT_TEMPERATURES

/**
 * Include capabilities in M115 output
 */
#define EXTENDED_CAPABILITIES_REPORT

/**
 * Disable all Volumetric extrusion options
 */
//#define NO_VOLUMETRICS

#if DISABLED(NO_VOLUMETRICS)
/**
   * Volumetric extrusion default state
   * Activate to make volumetric extrusion the default method,
   * with DEFAULT_NOMINAL_FILAMENT_DIA as the default diameter.
   *
   * M200 D0 to disable, M200 Dn to set a new diameter.
   */
//#define VOLUMETRIC_DEFAULT_ON
#endif

/**
 * Enable this option for a leaner build of Marlin that removes all
 * workspace offsets, simplifying coordinate transformations, leveling, etc.
 *
 *  - M206 and M428 are disabled.
 *  - G92 will revert to its behavior from Marlin 1.0.
 */
//#define NO_WORKSPACE_OFFSETS

/**
 * Set the number of proportional font spaces required to fill up a typical character space.
 * This can help to better align the output of commands like `G29 O` Mesh Output.
 *
 * For clients that use a fixed-width font (like OctoPrint), leave this set to 1.0.
 * Otherwise, adjust according to your client and font.
 */
#define PROPORTIONAL_FONT_RATIO 1.0

/**
 * Spend 28 bytes of SRAM to optimize the GCode parser
 */
#define FASTER_GCODE_PARSER

/**
 * CNC G-code options
 * Support CNC-style G-code dialects used by laser cutters, drawing machine cams, etc.
 * Note that G0 feedrates should be used with care for 3D printing (if used at all).
 * High feedrates may cause ringing and harm print quality.
 */
//#define PAREN_COMMENTS      // Support for parentheses-delimited comments
//#define GCODE_MOTION_MODES  // Remember the motion mode (G0 G1 G2 G3 G5 G38.X) and apply for X Y Z E F, etc.

// Enable and set a (default) feedrate for all G0 moves
//#define G0_FEEDRATE 3000 // (mm/m)
#ifdef G0_FEEDRATE
//#define VARIABLE_G0_FEEDRATE // The G0 feedrate is set by F in G0 motion mode
#endif

/**
 * G-code Macros
 *
 * Add G-codes M810-M819 to define and run G-code macros.
 * Macros are not saved to EEPROM.
 */
//#define GCODE_MACROS
#if ENABLED(GCODE_MACROS)
    #define GCODE_MACROS_SLOTS 5 // Up to 10 may be used
    #define GCODE_MACROS_SLOT_SIZE 50 // Maximum length of a single macro
#endif

/**
 * User-defined menu items that execute custom GCode
 */
//#define CUSTOM_USER_MENUS
#if ENABLED(CUSTOM_USER_MENUS)
    //#define CUSTOM_USER_MENU_TITLE "Custom Commands"
    #define USER_SCRIPT_DONE "M117 User Script Done"
    #define USER_SCRIPT_AUDIBLE_FEEDBACK
//#define USER_SCRIPT_RETURN  // Return to status screen after a script

    #define USER_DESC_1 "Home & UBL Info"
    #define USER_GCODE_1 "G28\nG29 W"

    #define USER_DESC_2 "Preheat for " PREHEAT_1_LABEL
    #define USER_GCODE_2 "M140 S" STRINGIFY(PREHEAT_1_TEMP_BED) "\nM104 S" STRINGIFY(PREHEAT_1_TEMP_HOTEND)

    #define USER_DESC_3 "Preheat for " PREHEAT_2_LABEL
    #define USER_GCODE_3 "M140 S" STRINGIFY(PREHEAT_2_TEMP_BED) "\nM104 S" STRINGIFY(PREHEAT_2_TEMP_HOTEND)

    #define USER_DESC_4 "Heat Bed/Home/Level"
    #define USER_GCODE_4 "M140 S" STRINGIFY(PREHEAT_2_TEMP_BED) "\nG28\nG29"

    #define USER_DESC_5 "Home & Info"
    #define USER_GCODE_5 "G28\nM503"
#endif

/**
 * Host Action Commands
 *
 * Define host streamer action commands in compliance with the standard.
 *
 * See https://reprap.org/wiki/G-code#Action_commands
 * Common commands ........ poweroff, pause, paused, resume, resumed, cancel
 * G29_RETRY_AND_RECOVER .. probe_rewipe, probe_failed
 *
 * Some features add reason codes to extend these commands.
 *
 * Host Prompt Support enables Marlin to use the host for user prompts so
 * filament runout and other processes can be managed from the host side.
 */
//#define HOST_ACTION_COMMANDS
#if ENABLED(HOST_ACTION_COMMANDS)
    #define HOST_PROMPT_SUPPORT
#endif

//===========================================================================
//====================== I2C Position Encoder Settings ======================
//===========================================================================

/**
 * I2C position encoders for closed loop control.
 * Developed by Chris Barr at Aus3D.
 *
 * Wiki: http://wiki.aus3d.com.au/Magnetic_Encoder
 * Github: https://github.com/Aus3D/MagneticEncoder
 *
 * Supplier: http://aus3d.com.au/magnetic-encoder-module
 * Alternative Supplier: http://reliabuild3d.com/
 *
 * Reliabuild encoders have been modified to improve reliability.
 */

//#define I2C_POSITION_ENCODERS
#if ENABLED(I2C_POSITION_ENCODERS)
    /**
     * The number of encoders installed
     * max of 5 encoders supported currently.
     */
    #define I2CPE_ENCODER_CNT 1
    #define I2CPE_ENC_1_ADDR I2CPE_PRESET_ADDR_X // I2C address of the encoder. 30-200.
    #define I2CPE_ENC_1_AXIS X_AXIS // Axis the encoder module is installed on.  <X|Y|Z|E>_AXIS.
    /**
     * Type of encoder:  I2CPE_ENC_TYPE_LINEAR -or- I2CPE_ENC_TYPE_ROTARY.
     */
    #define I2CPE_ENC_1_TYPE I2CPE_ENC_TYPE_LINEAR
    /**
     * 1024 for magnetic strips with 2mm poles; 2048 for
     * 1mm poles. For linear encoders this is ticks / mm,
     * for rotary encoders this is ticks / revolution.
     */
    #define I2CPE_ENC_1_TICKS_UNIT 2048
    /**
     * Only needed for rotary encoders; number of stepper
     * steps per full revolution (motor steps/rev * microstepping)
     */
    //#define I2CPE_ENC_1_TICKS_REV     (16 * 200)
    //#define I2CPE_ENC_1_INVERT // Invert the direction of axis travel.
    #define I2CPE_ENC_1_EC_METHOD I2CPE_ECM_MICROSTEP // Type of error error correction.
    /**
     * Threshold size for error (in mm) above which the
     * printer will attempt to correct the error; errors
     * smaller than this are ignored to minimize effects of
     * measurement noise / latency (filter).
     */
    #define I2CPE_ENC_1_EC_THRESH 0.10
    #define I2CPE_ENC_2_ADDR I2CPE_PRESET_ADDR_Y // Same as above, but for encoder 2.
    #define I2CPE_ENC_2_AXIS Y_AXIS
    #define I2CPE_ENC_2_TYPE I2CPE_ENC_TYPE_LINEAR
    #define I2CPE_ENC_2_TICKS_UNIT 2048
    //#define I2CPE_ENC_2_TICKS_REV   (16 * 200)
    //#define I2CPE_ENC_2_INVERT
    #define I2CPE_ENC_2_EC_METHOD I2CPE_ECM_MICROSTEP
    #define I2CPE_ENC_2_EC_THRESH 0.10

    #define I2CPE_ENC_3_ADDR I2CPE_PRESET_ADDR_Z // Encoder 3.  Add additional configuration options
    #define I2CPE_ENC_3_AXIS Z_AXIS // as above, or use defaults below.

    #define I2CPE_ENC_4_ADDR I2CPE_PRESET_ADDR_E // Encoder 4.
    #define I2CPE_ENC_4_AXIS E_AXIS

    #define I2CPE_ENC_5_ADDR 34 // Encoder 5.
    #define I2CPE_ENC_5_AXIS E_AXIS

    // Default settings for encoders which are enabled, but without settings configured above.
    #define I2CPE_DEF_TYPE I2CPE_ENC_TYPE_LINEAR
    #define I2CPE_DEF_ENC_TICKS_UNIT 2048
    #define I2CPE_DEF_TICKS_REV (16 * 200)
    #define I2CPE_DEF_EC_METHOD I2CPE_ECM_NONE
    #define I2CPE_DEF_EC_THRESH 0.1
    /**
     * Threshold size for error (in mm) error on any given
     * axis after which the printer will abort. Comment out to
     * disable abort behaviour.
     */
    //#define I2CPE_ERR_THRESH_ABORT  100.0
    /**
     * After an encoder fault, there must be no further fault
     * for this amount of time (in ms) before the encoder
     * is trusted again.
     */
    #define I2CPE_TIME_TRUSTED 10000
    /**
     * Position is checked every time a new command is executed from the buffer but during long moves,
     * this setting determines the minimum update time between checks. A value of 100 works well with
     * error rolling average when attempting to correct only for skips and not for vibration.
     */
    #define I2CPE_MIN_UPD_TIME_MS 4 // (ms) Minimum time between encoder checks.

    // Use a rolling average to identify persistant errors that indicate skips, as opposed to vibration and noise.
    #define I2CPE_ERR_ROLLING_AVERAGE

#endif // I2C_POSITION_ENCODERS

/**
 * MAX7219 Debug Matrix
 *
 * Add support for a low-cost 8x8 LED Matrix based on the Max7219 chip as a realtime status display.
 * Requires 3 signal wires. Some useful debug options are included to demonstrate its usage.
 */
//#define MAX7219_DEBUG
#if ENABLED(MAX7219_DEBUG)
    #define MAX7219_CLK_PIN 64
    #define MAX7219_DIN_PIN 57
    #define MAX7219_LOAD_PIN 44

    //#define MAX7219_GCODE          // Add the M7219 G-code to control the LED matrix
    #define MAX7219_INIT_TEST 2 // Do a test pattern at initialization (Set to 2 for spiral)
    #define MAX7219_NUMBER_UNITS 1 // Number of Max7219 units in chain.
    /**
     * Rotate the display clockwise (in multiples of +/- 90°)
     * connector at:  right=0   bottom=-90  top=90  left=180
     */
    #define MAX7219_ROTATE 0
    //#define MAX7219_REVERSE_ORDER  // The individual LED matrix units may be in reversed order

    /**
   * Sample debug features
   * If you add more debug displays, be careful to avoid conflicts!
   */
    #define MAX7219_DEBUG_PRINTER_ALIVE // Blink corner LED of 8x8 matrix to show that the firmware is functioning
    #define MAX7219_DEBUG_PLANNER_HEAD 3 // Show the planner queue head position on this and the next LED matrix row
    #define MAX7219_DEBUG_PLANNER_TAIL 5 // Show the planner queue tail position on this and the next LED matrix row
    /**
     * Show the current planner queue depth on this and the next LED matrix row
     * If you experience stuttering, reboots, etc. this option can reveal how
     * tweaks made to the configuration are affecting the printer in real-time.
     */
    #define MAX7219_DEBUG_PLANNER_QUEUE 0
#endif

/**
 * NanoDLP Sync support
 *
 * Add support for Synchronized Z moves when using with NanoDLP. G0/G1 axis moves will output "Z_move_comp"
 * string to enable synchronization with DLP projector exposure. This change will allow to use
 * [[WaitForDoneMessage]] instead of populating your gcode with M400 commands
 */
//#define NANODLP_Z_SYNC
#if ENABLED(NANODLP_Z_SYNC)
/**
 * Enables "Z_move_comp" output on any axis move.
 * Default behaviour is limited to Z axis only.
 */
//#define NANODLP_ALL_AXIS
#endif

/**
 * WiFi Support (Espressif ESP32 WiFi)
 */
//#define WIFISUPPORT
#if ENABLED(WIFISUPPORT)
    #define WIFI_SSID "Wifi SSID"
    #define WIFI_PWD "Wifi Password"
//#define WEBSUPPORT        // Start a webserver with auto-discovery
//#define OTASUPPORT        // Support over-the-air firmware updates
#endif

/**
 * Prusa Multi-Material Unit v2
 * Enable in Configuration.h
 */
#if ENABLED(PRUSA_MMU2)

    // Serial port used for communication with MMU2.
    // For AVR enable the UART port used for the MMU. (e.g., internalSerial)
    // For 32-bit boards check your HAL for available serial ports. (e.g., Serial2)
    #define INTERNAL_SERIAL_PORT 2
    #define MMU2_SERIAL internalSerial

    // Use hardware reset for MMU if a pin is defined for it
    //#define MMU2_RST_PIN 23

    // Enable if the MMU2 has 12V stepper motors (MMU2 Firmware 1.0.2 and up)
    //#define MMU2_MODE_12V

    // G-code to execute when MMU2 F.I.N.D.A. probe detects filament runout
    #define MMU2_FILAMENT_RUNOUT_SCRIPT "M600"

    // Add an LCD menu for MMU2
    //#define MMU2_MENUS
    #if ENABLED(MMU2_MENUS)
        // Settings for filament load / unload from the LCD menu.
        // This is for Prusa MK3-style extruders. Customize for your hardware.
        #define MMU2_FILAMENTCHANGE_EJECT_FEED 80.0
        #define MMU2_LOAD_TO_NOZZLE_SEQUENCE \
            { 7.2, 562 },                    \
                { 14.4, 871 },               \
                { 36.0, 1393 },              \
                { 14.4, 871 },               \
            { 50.0, 198 }

        #define MMU2_RAMMING_SEQUENCE \
            { 1.0, 1000 },            \
                { 1.0, 1500 },        \
                { 2.0, 2000 },        \
                { 1.5, 3000 },        \
                { 2.5, 4000 },        \
                { -15.0, 5000 },      \
                { -14.0, 1200 },      \
                { -6.0, 600 },        \
                { 10.0, 700 },        \
                { -10.0, 400 },       \
            { -50.0, 2000 }

    #endif

//#define MMU2_DEBUG  // Write debug info to serial output

#endif // PRUSA_MMU2

/**
 * Advanced Print Counter settings
 */
#if ENABLED(PRINTCOUNTER)
    #define SERVICE_WARNING_BUZZES 3
// Activate up to 3 service interval watchdogs
//#define SERVICE_NAME_1      "Service S"
//#define SERVICE_INTERVAL_1  100 // print hours
//#define SERVICE_NAME_2      "Service L"
//#define SERVICE_INTERVAL_2  200 // print hours
//#define SERVICE_NAME_3      "Service 3"
//#define SERVICE_INTERVAL_3    1 // print hours
#endif

// Prusa M73 implementation
#define M73_PRUSA

// Enable SDCARD gcodes M20 - M30
#define SDCARD_GCODES

// @section develop

/**
 * M43 - display pin status, watch pins for changes, watch endstops & toggle LED, Z servo probe test, toggle pins
 */
//#define PINS_DEBUGGING

// Enable Marlin dev mode which adds some special commands
//#define MARLIN_DEV_MODE

/**
 * M862.x support for print checking Q commands (P are always supported)
 */
#define PRINT_CHECKING_Q_CMDS


/**
 * Enable PID autotune
 **/
#define PID_AUTOTUNE
