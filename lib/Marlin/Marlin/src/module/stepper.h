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

// Helper class to disable the STEP ISR
class [[nodiscard]] StepIsrDisabler {
    bool old_step_isr_state;

public:
    StepIsrDisabler()
        : old_step_isr_state { STEPPER_ISR_ENABLED() } {
        if (old_step_isr_state) {
            DISABLE_STEPPER_DRIVER_INTERRUPT();
        }
    }

    ~StepIsrDisabler() {
        if (old_step_isr_state) {
            ENABLE_STEPPER_DRIVER_INTERRUPT();
        }
    }

    StepIsrDisabler(const StepIsrDisabler &) = delete;
    StepIsrDisabler &operator=(const StepIsrDisabler &) = delete;
};

//
// Stepper class definition
//
class Stepper {

public:
private:
    static uint8_t last_direction_bits; // The next stepping-bits to be output
    static uint8_t axis_did_move; // Last Movement in the given direction is not null, as computed when the last movement was fetched from planner

    //
    // Exact steps at which an endstop was triggered
    //
    static xyz_long_t endstops_trigsteps;

    //
    // Positions of stepper motors, in step units
    //
    static xyze_long_t count_position; // Current position (relative to home origin)
    static xyze_long_t count_position_from_startup; // Current position (absolute)
    static xyze_long_t count_position_last_block; // Position (relative to home origin) at the end
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
    static inline bool is_awake() { return STEPPER_ISR_ENABLED(); }

    static inline bool suspend() {
        const bool awake = is_awake();
        if (awake) {
            DISABLE_STEPPER_DRIVER_INTERRUPT();
        }
        return awake;
    }

    // Get the position of a stepper, in steps
    static int32_t position(const AxisEnum axis) {
        return count_position[axis];
    }
    static int32_t position_from_startup(const AxisEnum axis) {
        return count_position_from_startup[axis];
    }

    // Report the positions of the steppers, in steps
    static void report_positions();

    // The direction of a single motor
    FORCE_INLINE static bool motor_direction(const AxisEnum axis) { return TEST(last_direction_bits, axis); }

    // The last movement direction was not null on the specified axis. Note that motor direction is not necessarily the same.
    FORCE_INLINE static bool axis_is_moving(const AxisEnum axis) { return TEST(axis_did_move, axis); }

    // Handle a triggered endstop
    static void endstop_triggered(const AxisEnum axis);

    // Triggered position of an axis in steps
    static int32_t triggered_position(const AxisEnum axis) {
        return endstops_trigsteps[axis];
    }

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

    // Set the current position in steps for all axes at once
    static inline void set_position(const int32_t &a, const int32_t &b, const int32_t &c, const int32_t &e) {
        planner.synchronize();
        StepIsrDisabler step_guard;
        _set_position(a, b, c, e);
    }
    static inline void set_position(const xyze_long_t &abce) { set_position(abce.a, abce.b, abce.c, abce.e); }

    static inline void set_axis_position(const AxisEnum a, const int32_t &v) {
        planner.synchronize();
        count_position[a] = v;
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

        if (speed > 0) {
            last_direction_bits |= axis_mask;
        } else {
            last_direction_bits &= ~axis_mask;
        }
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
