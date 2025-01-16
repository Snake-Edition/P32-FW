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
 * stepper.h - stepper motor driver: executes motion plans of planner.c using the stepper motors
 * Derived from Grbl
 *
 * Copyright (c) 2009-2011 Simen Svale Skogsrud
 *
 * Grbl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Grbl is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Grbl.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "../inc/MarlinConfig.h"

#include "planner.h"
#include "stepper/indirection.h"
#ifdef __AVR__
  #include "speed_lookuptable.h"
#endif

// Disable multiple steps per ISR
//#define DISABLE_MULTI_STEPPING

//
// Estimate the amount of time the Stepper ISR will take to execute
//

#ifdef CPU_32_BIT

  // The base ISR takes 792 cycles
  #define ISR_BASE_CYCLES  792UL

  // S curve interpolation adds 40 cycles
  #if ENABLED(S_CURVE_ACCELERATION)
    #define ISR_S_CURVE_CYCLES 40UL
  #else
    #define ISR_S_CURVE_CYCLES 0UL
  #endif

  // Stepper Loop base cycles
  #define ISR_LOOP_BASE_CYCLES 4UL

  // To start the step pulse, in the worst case takes
  #define ISR_START_STEPPER_CYCLES 13UL

  // And each stepper (start + stop pulse) takes in worst case
  #define ISR_STEPPER_CYCLES 16UL

#else

  // The base ISR takes 752 cycles
  #define ISR_BASE_CYCLES  752UL

  // S curve interpolation adds 160 cycles
  #if ENABLED(S_CURVE_ACCELERATION)
    #define ISR_S_CURVE_CYCLES 160UL
  #else
    #define ISR_S_CURVE_CYCLES 0UL
  #endif

  // Stepper Loop base cycles
  #define ISR_LOOP_BASE_CYCLES 32UL

  // To start the step pulse, in the worst case takes
  #define ISR_START_STEPPER_CYCLES 57UL

  // And each stepper (start + stop pulse) takes in worst case
  #define ISR_STEPPER_CYCLES 88UL

#endif

// Add time for each stepper
#if HAS_X_STEP
  #define ISR_START_X_STEPPER_CYCLES ISR_START_STEPPER_CYCLES
  #define ISR_X_STEPPER_CYCLES       ISR_STEPPER_CYCLES
#else
  #define ISR_START_X_STEPPER_CYCLES 0UL
  #define ISR_X_STEPPER_CYCLES       0UL
#endif
#if HAS_Y_STEP
  #define ISR_START_Y_STEPPER_CYCLES ISR_START_STEPPER_CYCLES
  #define ISR_Y_STEPPER_CYCLES       ISR_STEPPER_CYCLES
#else
  #define ISR_START_Y_STEPPER_CYCLES 0UL
  #define ISR_Y_STEPPER_CYCLES       0UL
#endif
#if HAS_Z_STEP
  #define ISR_START_Z_STEPPER_CYCLES ISR_START_STEPPER_CYCLES
  #define ISR_Z_STEPPER_CYCLES       ISR_STEPPER_CYCLES
#else
  #define ISR_START_Z_STEPPER_CYCLES 0UL
  #define ISR_Z_STEPPER_CYCLES       0UL
#endif

// E is always interpolated, even for mixing extruders
#define ISR_START_E_STEPPER_CYCLES   ISR_START_STEPPER_CYCLES
#define ISR_E_STEPPER_CYCLES         ISR_STEPPER_CYCLES

// Calculate the minimum time to start all stepper pulses in the ISR loop
#define MIN_ISR_START_LOOP_CYCLES (ISR_START_X_STEPPER_CYCLES + ISR_START_Y_STEPPER_CYCLES + ISR_START_Z_STEPPER_CYCLES + ISR_START_E_STEPPER_CYCLES + ISR_START_MIXING_STEPPER_CYCLES)

// And the total minimum loop time, not including the base
#define MIN_ISR_LOOP_CYCLES (ISR_X_STEPPER_CYCLES + ISR_Y_STEPPER_CYCLES + ISR_Z_STEPPER_CYCLES + ISR_E_STEPPER_CYCLES + ISR_MIXING_STEPPER_CYCLES)

// Calculate the minimum MPU cycles needed per pulse to enforce, limited to the max stepper rate
#define _MIN_STEPPER_PULSE_CYCLES(N) _MAX(uint32_t((F_CPU) / (MAXIMUM_STEPPER_RATE)), ((F_CPU) / 500000UL) * (N))
#if MINIMUM_STEPPER_PULSE
  #define MIN_STEPPER_PULSE_CYCLES _MIN_STEPPER_PULSE_CYCLES(uint32_t(MINIMUM_STEPPER_PULSE))
#elif HAS_DRIVER(LV8729)
  #define MIN_STEPPER_PULSE_CYCLES uint32_t((((F_CPU) - 1) / 2000000) + 1) // 0.5µs, aka 500ns
#else
  #define MIN_STEPPER_PULSE_CYCLES _MIN_STEPPER_PULSE_CYCLES(1UL)
#endif

// Calculate the minimum ticks of the PULSE timer that must elapse with the step pulse enabled
// adding the "start stepper pulse" code section execution cycles to account for that not all
// pulses start at the beginning of the loop, so an extra time must be added to compensate so
// the last generated pulse (usually the extruder stepper) has the right length
#if HAS_DRIVER(LV8729) && MINIMUM_STEPPER_PULSE == 0
  #define MIN_PULSE_TICKS ((((PULSE_TIMER_TICKS_PER_US) + 1) / 2) + ((MIN_ISR_START_LOOP_CYCLES) / uint32_t(PULSE_TIMER_PRESCALE)))
#else
  #define MIN_PULSE_TICKS (((PULSE_TIMER_TICKS_PER_US) * uint32_t(MINIMUM_STEPPER_PULSE)) + ((MIN_ISR_START_LOOP_CYCLES) / uint32_t(PULSE_TIMER_PRESCALE)))
#endif

// Calculate the extra ticks of the PULSE timer between step pulses
#define ADDED_STEP_TICKS (((MIN_STEPPER_PULSE_CYCLES) / (PULSE_TIMER_PRESCALE)) - (MIN_PULSE_TICKS))

// But the user could be enforcing a minimum time, so the loop time is
#define ISR_LOOP_CYCLES (ISR_LOOP_BASE_CYCLES + _MAX(MIN_STEPPER_PULSE_CYCLES, MIN_ISR_LOOP_CYCLES))

// Now estimate the total ISR execution time in cycles given a step per ISR multiplier
#define ISR_EXECUTION_CYCLES(R) (((ISR_BASE_CYCLES + ISR_S_CURVE_CYCLES + (ISR_LOOP_CYCLES) * (R) + ISR_LA_BASE_CYCLES + ISR_LA_LOOP_CYCLES)) / (R))

// The maximum allowable stepping frequency when doing x128-x1 stepping (in Hz)
#define MAX_STEP_ISR_FREQUENCY_128X ((F_CPU) / ISR_EXECUTION_CYCLES(128))
#define MAX_STEP_ISR_FREQUENCY_64X  ((F_CPU) / ISR_EXECUTION_CYCLES(64))
#define MAX_STEP_ISR_FREQUENCY_32X  ((F_CPU) / ISR_EXECUTION_CYCLES(32))
#define MAX_STEP_ISR_FREQUENCY_16X  ((F_CPU) / ISR_EXECUTION_CYCLES(16))
#define MAX_STEP_ISR_FREQUENCY_8X   ((F_CPU) / ISR_EXECUTION_CYCLES(8))
#define MAX_STEP_ISR_FREQUENCY_4X   ((F_CPU) / ISR_EXECUTION_CYCLES(4))
#define MAX_STEP_ISR_FREQUENCY_2X   ((F_CPU) / ISR_EXECUTION_CYCLES(2))
#define MAX_STEP_ISR_FREQUENCY_1X   ((F_CPU) / ISR_EXECUTION_CYCLES(1))

// The minimum allowable frequency for step smoothing will be 1/10 of the maximum nominal frequency (in Hz)
#define MIN_STEP_ISR_FREQUENCY MAX_STEP_ISR_FREQUENCY_1X

//
// Stepper class definition
//
class Stepper {

  public:

  private:

    static uint8_t last_direction_bits,     // The next stepping-bits to be output
                   axis_did_move;           // Last Movement in the given direction is not null, as computed when the last movement was fetched from planner

    //
    // Exact steps at which an endstop was triggered
    //
    static xyz_long_t endstops_trigsteps;

    //
    // Positions of stepper motors, in step units
    //
    static xyze_long_t count_position;              // Current position (relative to home origin)
    static xyze_long_t count_position_from_startup; // Current position (absolute)
    static xyze_long_t count_position_last_block;   // Position (relative to home origin) at the end
                                                    // of the last block

    //
    // Current direction of stepper motors (+1 or -1)
    //
    static xyze_int8_t count_direction;

  public:

    //
    // Constructor / initializer
    //
    Stepper() {};

    // Initialize stepper hardware
    static void init();

    // Interrupt Service Routine and phases

    // The stepper subsystem goes to sleep when it runs out of things to execute.
    // Call this to notify the subsystem that it is time to go to work.
    static inline void wake_up() { ENABLE_STEPPER_DRIVER_INTERRUPT(); }

    static inline bool is_awake() { return STEPPER_ISR_ENABLED(); }

    static inline bool suspend() {
      const bool awake = is_awake();
      if (awake) DISABLE_STEPPER_DRIVER_INTERRUPT();
      return awake;
    }

    // Get the position of a stepper, in steps
    static int32_t position(const AxisEnum axis);
    static int32_t position_from_startup(const AxisEnum axis);

    // Report the positions of the steppers, in steps
    static void report_positions();

    // Force any planned move to start immediately
    static inline void start_moving() {
      if (planner.movesplanned()) {
        suspend();
        planner.delay_before_delivering = 0;
        // TODO: implement this for PreciseStepping
        //if (!current_block) isr(); // zero-wait
        wake_up();
      }
    }

    // The direction of a single motor
    FORCE_INLINE static bool motor_direction(const AxisEnum axis) { return TEST(last_direction_bits, axis); }

    // The last movement direction was not null on the specified axis. Note that motor direction is not necessarily the same.
    FORCE_INLINE static bool axis_is_moving(const AxisEnum axis) { return TEST(axis_did_move, axis); }

    // Handle a triggered endstop
    static void endstop_triggered(const AxisEnum axis);

    // Triggered position of an axis in steps
    static int32_t triggered_position(const AxisEnum axis);

    #if HAS_DRIVER(TMC2130) || HAS_DRIVER(TMC2209)
      static void microstep_ms(const uint8_t driver, const int8_t ms1, const int8_t ms2, const int8_t ms3);
      static void microstep_mode(const uint8_t driver, const uint8_t stepping);
      static void microstep_readings();
    #endif

    #if HAS_EXTRA_ENDSTOPS || ENABLED(Z_STEPPER_AUTO_ALIGN)
      FORCE_INLINE static void set_separate_multi_axis(const bool state) { separate_multi_axis = state; }
    #endif
    #if ENABLED(X_DUAL_ENDSTOPS)
      FORCE_INLINE static void set_x_lock(const bool state) { locked_X_motor = state; }
      FORCE_INLINE static void set_x2_lock(const bool state) { locked_X2_motor = state; }
    #endif
    #if ENABLED(Y_DUAL_ENDSTOPS)
      FORCE_INLINE static void set_y_lock(const bool state) { locked_Y_motor = state; }
      FORCE_INLINE static void set_y2_lock(const bool state) { locked_Y2_motor = state; }
    #endif
    #if Z_MULTI_ENDSTOPS || (ENABLED(Z_STEPPER_AUTO_ALIGN) && Z_MULTI_STEPPER_DRIVERS)
      FORCE_INLINE static void set_z_lock(const bool state) { locked_Z_motor = state; }
      FORCE_INLINE static void set_z2_lock(const bool state) { locked_Z2_motor = state; }
    #endif
    #if ENABLED(Z_TRIPLE_ENDSTOPS) || BOTH(Z_STEPPER_AUTO_ALIGN, Z_TRIPLE_STEPPER_DRIVERS)
      FORCE_INLINE static void set_z3_lock(const bool state) { locked_Z3_motor = state; }
    #endif

    #if ENABLED(BABYSTEPPING)
      static void babystep(const AxisEnum axis, const bool direction); // perform a short step with a single stepper motor, outside of any convention
    #endif

    // Set the current position in steps
    static inline void set_position(const int32_t &a, const int32_t &b, const int32_t &c, const int32_t &e) {
      planner.synchronize();
      const bool was_enabled = suspend();
      _set_position(a, b, c, e);
      if (was_enabled) wake_up();
    }
    static inline void set_position(const xyze_long_t &abce) { set_position(abce.a, abce.b, abce.c, abce.e); }

    static inline void set_axis_position(const AxisEnum a, const int32_t &v) {
      planner.synchronize();

      #ifdef __AVR__
        // Protect the access to the position. Only required for AVR, as
        //  any 32bit CPU offers atomic access to 32bit variables
        const bool was_enabled = suspend();
      #endif

      count_position[a] = v;

      #ifdef __AVR__
        // Reenable Stepper ISR
        if (was_enabled) wake_up();
      #endif
    }

    // Set direction bits for all steppers
    static void set_directions();

    // Return ratio of completed steps of current block (call within ISR context)
    static float segment_progress();

    static long get_axis_steps(const AxisEnum a) {
      return count_position[a];
    }

    static long get_axis_steps_from_startup(const AxisEnum a) {
      return count_position_from_startup[a];
    }

    static void set_axis_steps(const AxisEnum a, long steps_made) {
      count_position[a] = steps_made;
    }

    static void set_axis_steps_from_startup(const AxisEnum a, long steps_made) {
      count_position_from_startup[a] = steps_made;
    }

    // Set axis usage and direction bits based on physical direction
    static void report_axis_movement(AxisEnum a, float speed) {
      uint8_t axis_mask = 1 << a;
      axis_did_move |= axis_mask;

      if (speed > 0)
        last_direction_bits |= axis_mask;
      else
        last_direction_bits &= ~axis_mask;
    }

    // Return true if the physical axis direction is inverted
    static bool is_axis_inverted(AxisEnum a);

private:
    // Set the current position in steps
    static void _set_position(const int32_t &a, const int32_t &b, const int32_t &c, const int32_t &e);
    FORCE_INLINE static void _set_position(const abce_long_t &spos) { _set_position(spos.a, spos.b, spos.c, spos.e); }

    friend class PreciseStepping;
};

extern Stepper stepper;
