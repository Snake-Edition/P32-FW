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

/**
 * stepper.cpp - A singleton object to execute motion plans using stepper motors
 * Marlin Firmware
 *
 * Derived from Grbl
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

/**
 * Timer calculations informed by the 'RepRap cartesian firmware' by Zack Smith
 * and Philipp Tiefenbacher.
 */

/**
 *         __________________________
 *        /|                        |\     _________________         ^
 *       / |                        | \   /|               |\        |
 *      /  |                        |  \ / |               | \       s
 *     /   |                        |   |  |               |  \      p
 *    /    |                        |   |  |               |   \     e
 *   +-----+------------------------+---+--+---------------+----+    e
 *   |               BLOCK 1            |      BLOCK 2          |    d
 *
 *                           time ----->
 *
 *  The trapezoid is the shape the speed curve over time. It starts at block->initial_rate, accelerates
 *  first block->accelerate_until step_events_completed, then keeps going at constant speed until
 *  step_events_completed reaches block->decelerate_after after which it decelerates until the trapezoid generator is reset.
 *  The slope of acceleration is calculated using v = u + at where t is the accumulated timer values of the steps so far.
 */

/**
 * Marlin uses the Bresenham algorithm. For a detailed explanation of theory and
 * method see https://www.cs.helsinki.fi/group/goa/mallinnus/lines/bresenh.html
 */

/**
 * Jerk controlled movements planner added Apr 2018 by Eduardo José Tagle.
 * Equations based on Synthethos TinyG2 sources, but the fixed-point
 * implementation is new, as we are running the ISR with a variable period.
 * Also implemented the Bézier velocity curve evaluation in ARM assembler,
 * to avoid impacting ISR speed.
 */

#include "stepper.h"

Stepper stepper; // Singleton

#include "planner.h"
#include <timing_precise.hpp>

// public:

#if HAS_EXTRA_ENDSTOPS || ENABLED(Z_STEPPER_AUTO_ALIGN)
bool Stepper::separate_multi_axis = false;
#endif

// private:

uint8_t Stepper::last_direction_bits; // = 0
uint8_t Stepper::axis_did_move; // = 0

xyz_long_t Stepper::endstops_trigsteps;
xyze_long_t Stepper::count_position { 0 };
xyze_long_t Stepper::count_position_from_startup { 0 };
xyze_long_t Stepper::count_position_last_block { 0 };
xyze_int8_t Stepper::count_direction { 0 };

/**
 * Set the stepper direction of each axis
 *
 *   COREXY: X_AXIS=A_AXIS and Y_AXIS=B_AXIS
 *   COREXZ: X_AXIS=A_AXIS and Z_AXIS=C_AXIS
 *   COREYZ: Y_AXIS=B_AXIS and Z_AXIS=C_AXIS
 */
void Stepper::set_directions() {

#if MINIMUM_STEPPER_PRE_DIR_DELAY > 0
    delay_ns_precise<MINIMUM_STEPPER_PRE_DIR_DELAY>();
#endif

#define SET_STEP_DIR(A)                   \
    if (motor_direction(_AXIS(A))) {      \
        A##_APPLY_DIR(INVERT_##A##_DIR);  \
        count_direction[_AXIS(A)] = -1;   \
    } else {                              \
        A##_APPLY_DIR(!INVERT_##A##_DIR); \
        count_direction[_AXIS(A)] = 1;    \
    }

#if HAS_X_DIR
    SET_STEP_DIR(X); // A
#endif

#if HAS_Y_DIR
    SET_STEP_DIR(Y); // B
#endif

#if HAS_Z_DIR
    SET_STEP_DIR(Z); // C
#endif

    if (motor_direction(E_AXIS)) {
        REV_E_DIR(stepper_extruder);
        count_direction.e = -1;
    } else {
        NORM_E_DIR(stepper_extruder);
        count_direction.e = 1;
    }

// A small delay may be needed after changing direction
#if MINIMUM_STEPPER_POST_DIR_DELAY > 0
    delay_ns_precise<MINIMUM_STEPPER_POST_DIR_DELAY>();
#endif
}

// Return ratio of completed steps of current block (call within ISR context)
float Stepper::segment_progress() {
    const block_t *current_block = planner.get_current_processed_block();
    if (!current_block || !current_block->mstep_event_count) {
        return NAN;
    }

    abce_ulong_t planned_msteps = current_block->msteps;
    xyze_long_t done_msteps = (count_position - count_position_last_block) * PLANNER_STEPS_MULTIPLIER;

    float planned;
    float done;

    if (planned_msteps.a || planned_msteps.b || planned_msteps.c) {
        // explicitly ignore extruder
        planned = (float)(planned_msteps.a + planned_msteps.b + planned_msteps.c);
        done = (float)(abs(done_msteps.a) + abs(done_msteps.b) + abs(done_msteps.c));
    } else {
        planned = (float)(current_block->mstep_event_count);
        done = (float)(abs(done_msteps.a) + abs(done_msteps.b) + abs(done_msteps.c) + abs(done_msteps.e));
    }

    return done / planned;
}

bool Stepper::is_axis_inverted(AxisEnum axis) {
    switch (axis) {
    case X_AXIS:
        return INVERT_X_DIR;
    case Y_AXIS:
        return INVERT_Y_DIR;
    case Z_AXIS:
        return INVERT_Z_DIR;
    default:
        return false; // other axes cannot be inverted
    }
}

#define _APPLY_STEP(AXIS)      AXIS##_STEP_WRITE
#define _INVERT_STEP_PIN(AXIS) INVERT_##AXIS##_STEP_PIN

void Stepper::init() {

// Init Dir Pins
#if HAS_X_DIR
    X_DIR_INIT;
#endif
#if HAS_X2_DIR
    X2_DIR_INIT;
#endif
#if HAS_Y_DIR
    Y_DIR_INIT;
    #if ENABLED(Y_DUAL_STEPPER_DRIVERS) && HAS_Y2_DIR
    Y2_DIR_INIT;
    #endif
#endif
#if HAS_Z_DIR
    Z_DIR_INIT;
    #if Z_MULTI_STEPPER_DRIVERS && HAS_Z2_DIR
    Z2_DIR_INIT;
    #endif
    #if ENABLED(Z_TRIPLE_STEPPER_DRIVERS) && HAS_Z3_DIR
    Z3_DIR_INIT;
    #endif
#endif
#if HAS_E0_DIR
    E0_DIR_INIT;
#endif
#if HAS_E1_DIR
    E1_DIR_INIT;
#endif
#if HAS_E2_DIR
    E2_DIR_INIT;
#endif
#if HAS_E3_DIR
    E3_DIR_INIT;
#endif
#if HAS_E4_DIR
    E4_DIR_INIT;
#endif
#if HAS_E5_DIR
    E5_DIR_INIT;
#endif

// Init Enable Pins - steppers default to disabled.
#if HAS_X_ENABLE
    X_ENABLE_INIT;
    if (!X_ENABLE_ON) {
        X_ENABLE_WRITE(HIGH);
    }
    #if EITHER(DUAL_X_CARRIAGE, X_DUAL_STEPPER_DRIVERS) && HAS_X2_ENABLE
    X2_ENABLE_INIT;
    if (!X_ENABLE_ON) {
        X2_ENABLE_WRITE(HIGH);
    }
    #endif
#endif
#if HAS_Y_ENABLE
    Y_ENABLE_INIT;
    if (!Y_ENABLE_ON) {
        Y_ENABLE_WRITE(HIGH);
    }
    #if ENABLED(Y_DUAL_STEPPER_DRIVERS) && HAS_Y2_ENABLE
    Y2_ENABLE_INIT;
    if (!Y_ENABLE_ON) {
        Y2_ENABLE_WRITE(HIGH);
    }
    #endif
#endif
#if HAS_Z_ENABLE
    Z_ENABLE_INIT;
    if (!Z_ENABLE_ON) {
        Z_ENABLE_WRITE(HIGH);
    }
    #if Z_MULTI_STEPPER_DRIVERS && HAS_Z2_ENABLE
    Z2_ENABLE_INIT;
    if (!Z_ENABLE_ON) {
        Z2_ENABLE_WRITE(HIGH);
    }
    #endif
    #if ENABLED(Z_TRIPLE_STEPPER_DRIVERS) && HAS_Z3_ENABLE
    Z3_ENABLE_INIT;
    if (!Z_ENABLE_ON) {
        Z3_ENABLE_WRITE(HIGH);
    }
    #endif
#endif
#if HAS_E0_ENABLE
    E0_ENABLE_INIT;
    if (!E_ENABLE_ON) {
        E0_ENABLE_WRITE(HIGH);
    }
#endif
#if HAS_E1_ENABLE
    E1_ENABLE_INIT;
    if (!E_ENABLE_ON) {
        E1_ENABLE_WRITE(HIGH);
    }
#endif
#if HAS_E2_ENABLE
    E2_ENABLE_INIT;
    if (!E_ENABLE_ON) {
        E2_ENABLE_WRITE(HIGH);
    }
#endif
#if HAS_E3_ENABLE
    E3_ENABLE_INIT;
    if (!E_ENABLE_ON) {
        E3_ENABLE_WRITE(HIGH);
    }
#endif
#if HAS_E4_ENABLE
    E4_ENABLE_INIT;
    if (!E_ENABLE_ON) {
        E4_ENABLE_WRITE(HIGH);
    }
#endif
#if HAS_E5_ENABLE
    E5_ENABLE_INIT;
    if (!E_ENABLE_ON) {
        E5_ENABLE_WRITE(HIGH);
    }
#endif

#define _STEP_INIT(AXIS)           AXIS##_STEP_INIT
#define _WRITE_STEP(AXIS, HIGHLOW) AXIS##_STEP_WRITE(HIGHLOW)
#define _DISABLE(AXIS)             disable_##AXIS()

#define AXIS_INIT(AXIS, PIN) \
    _STEP_INIT(AXIS);        \
    _WRITE_STEP(AXIS, _INVERT_STEP_PIN(PIN))

#define E_AXIS_INIT(NUM) AXIS_INIT(E##NUM, E)

// Init Step Pins
#if HAS_X_STEP
    #if EITHER(X_DUAL_STEPPER_DRIVERS, DUAL_X_CARRIAGE)
    X2_STEP_INIT;
    X2_STEP_WRITE(INVERT_X_STEP_PIN);
    #endif
    AXIS_INIT(X, X);
    #if ENABLED(XY_LINKED_ENABLE)
    _DISABLE(XY);
    #else
    _DISABLE(X);
    #endif
#endif

#if HAS_Y_STEP
    #if ENABLED(Y_DUAL_STEPPER_DRIVERS)
    Y2_STEP_INIT;
    Y2_STEP_WRITE(INVERT_Y_STEP_PIN);
    #endif
    AXIS_INIT(Y, Y);
    #if ENABLED(XY_LINKED_ENABLE)
    _DISABLE(XY);
    #else
    _DISABLE(Y);
    #endif
#endif

#if HAS_Z_STEP
    #if Z_MULTI_STEPPER_DRIVERS
    Z2_STEP_INIT;
    Z2_STEP_WRITE(INVERT_Z_STEP_PIN);
    #endif
    #if ENABLED(Z_TRIPLE_STEPPER_DRIVERS)
    Z3_STEP_INIT;
    Z3_STEP_WRITE(INVERT_Z_STEP_PIN);
    #endif
    AXIS_INIT(Z, Z);
    _DISABLE(Z);
#endif

#if E_STEPPERS > 0 && HAS_E0_STEP
    E_AXIS_INIT(0);
#endif
#if E_STEPPERS > 1 && HAS_E1_STEP
    E_AXIS_INIT(1);
#endif
#if E_STEPPERS > 2 && HAS_E2_STEP
    E_AXIS_INIT(2);
#endif
#if E_STEPPERS > 3 && HAS_E3_STEP
    E_AXIS_INIT(3);
#endif
#if E_STEPPERS > 4 && HAS_E4_STEP
    E_AXIS_INIT(4);
#endif
#if E_STEPPERS > 5 && HAS_E5_STEP
    E_AXIS_INIT(5);
#endif

    // Init direction bits for first moves
    last_direction_bits = 0
        | (INVERT_X_DIR ? _BV(X_AXIS) : 0)
        | (INVERT_Y_DIR ? _BV(Y_AXIS) : 0)
        | (INVERT_Z_DIR ? _BV(Z_AXIS) : 0);

    set_directions();
}

/**
 * Set the stepper positions directly in steps
 *
 * The input is based on the typical per-axis XYZ steps.
 * For CORE machines XYZ needs to be translated to ABC.
 *
 * This allows get_axis_position_mm to correctly
 * derive the current XYZ position later on.
 */
void Stepper::_set_position(const int32_t &a, const int32_t &b, const int32_t &c, const int32_t &e) {
#if CORE_IS_XY
    // corexy positioning
    // these equations follow the form of the dA and dB equations on http://www.corexy.com/theory.html
    count_position.set(a + b, CORESIGN(a - b), c);
#elif CORE_IS_XZ
    // corexz planning
    count_position.set(a + c, b, CORESIGN(a - c));
#elif CORE_IS_YZ
    // coreyz planning
    count_position.set(a, b + c, CORESIGN(b - c));
#else
    // default non-h-bot planning
    count_position.set(a, b, c);
#endif
    count_position.e = e;
}

// Signal endstops were triggered - This function can be called from
// an ISR context  (Temperature, Stepper or limits ISR).
void Stepper::endstop_triggered(const AxisEnum axis) {
    PreciseStepping::quick_stop();

    endstops_trigsteps[axis] = (
#if IS_CORE
        (axis == CORE_AXIS_2
                ? CORESIGN(count_position[CORE_AXIS_1] - count_position[CORE_AXIS_2])
                : count_position[CORE_AXIS_1] + count_position[CORE_AXIS_2])
        * 0.5f
#else // !IS_CORE
        count_position[axis]
#endif
    );
}

void Stepper::report_positions() {
    // Note that the reported position is not atomic/synchronous for all axes
    // to avoid locking the step ISR
    const xyz_long_t pos = count_position;

#if CORE_IS_XY || CORE_IS_XZ || ENABLED(DELTA) || IS_SCARA
    SERIAL_ECHOPAIR(MSG_COUNT_A, pos.x, " B:", pos.y);
#else
    SERIAL_ECHOPAIR(MSG_COUNT_X, pos.x, " Y:", pos.y);
#endif
#if CORE_IS_XZ || CORE_IS_YZ || ENABLED(DELTA)
    SERIAL_ECHOLNPAIR(" C:", pos.z);
#else
    SERIAL_ECHOLNPAIR(" Z:", pos.z);
#endif
}

#if ENABLED(BABYSTEPPING)

    #if MINIMUM_STEPPER_PULSE
        #define STEP_PULSE_CYCLES ((MINIMUM_STEPPER_PULSE)*CYCLES_PER_MICROSECOND)
    #else
        #define STEP_PULSE_CYCLES 0
    #endif

    #if ENABLED(DELTA)
        #define CYCLES_EATEN_BABYSTEP (2 * 15)
    #else
        #define CYCLES_EATEN_BABYSTEP 0
    #endif
    #define EXTRA_CYCLES_BABYSTEP (STEP_PULSE_CYCLES - (CYCLES_EATEN_BABYSTEP))

    #define _ENABLE(AXIS)            enable_##AXIS()
    #define _READ_DIR(AXIS)          AXIS##_DIR_READ()
    #define _INVERT_DIR(AXIS)        INVERT_##AXIS##_DIR
    #define _APPLY_DIR(AXIS, INVERT) AXIS##_APPLY_DIR(INVERT)

    #if EXTRA_CYCLES_BABYSTEP > 20
        #define _SAVE_START const hal_timer_t pulse_start = HAL_timer_get_count(PULSE_TIMER_NUM)
        #define _PULSE_WAIT                                                                                                                      \
            while (EXTRA_CYCLES_BABYSTEP > (uint32_t)(HAL_timer_get_count(PULSE_TIMER_NUM) - pulse_start) * (PULSE_TIMER_PRESCALE)) { /* nada */ \
            }
    #else
        #define _SAVE_START NOOP
        #if EXTRA_CYCLES_BABYSTEP > 0
            #define _PULSE_WAIT delay_ns_precise<EXTRA_CYCLES_BABYSTEP * NANOSECONDS_PER_CYCLE>()
        #elif ENABLED(DELTA)
            #define _PULSE_WAIT delay_us_precise<2>()
        #elif STEP_PULSE_CYCLES > 0
            #define _PULSE_WAIT NOOP
        #else
            #define _PULSE_WAIT delay_us_precise<4>()
        #endif
    #endif

    #define BABYSTEP_AXIS(AXIS, INVERT, DIR)                    \
        {                                                       \
            const uint8_t old_dir = _READ_DIR(AXIS);            \
            _ENABLE(AXIS);                                      \
            delay_ns_precise<MINIMUM_STEPPER_PRE_DIR_DELAY>();  \
            _APPLY_DIR(AXIS, _INVERT_DIR(AXIS) ^ DIR ^ INVERT); \
            delay_ns_precise<MINIMUM_STEPPER_POST_DIR_DELAY>(); \
            _SAVE_START;                                        \
            _APPLY_STEP(AXIS)                                   \
            (!_INVERT_STEP_PIN(AXIS));                          \
            _PULSE_WAIT;                                        \
            _APPLY_STEP(AXIS)                                   \
            (_INVERT_STEP_PIN(AXIS));                           \
            _APPLY_DIR(AXIS, old_dir);                          \
        }

// MUST ONLY BE CALLED BY AN ISR,
// No other ISR should ever interrupt this!
void Stepper::babystep(const AxisEnum axis, const bool direction) {
    CRITICAL_SECTION_START;

    switch (axis) {

    #if ENABLED(BABYSTEP_XY)

    case X_AXIS:
        #if CORE_IS_XY
        BABYSTEP_AXIS(X, false, direction);
        BABYSTEP_AXIS(Y, false, direction);
        #elif CORE_IS_XZ
        BABYSTEP_AXIS(X, false, direction);
        BABYSTEP_AXIS(Z, false, direction);
        #else
        BABYSTEP_AXIS(X, false, direction);
        #endif
        break;

    case Y_AXIS:
        #if CORE_IS_XY
        BABYSTEP_AXIS(X, false, direction);
        BABYSTEP_AXIS(Y, false, direction ^ (CORESIGN(1) < 0));
        #elif CORE_IS_YZ
        BABYSTEP_AXIS(Y, false, direction);
        BABYSTEP_AXIS(Z, false, direction ^ (CORESIGN(1) < 0));
        #else
        BABYSTEP_AXIS(Y, false, direction);
        #endif
        break;

    #endif

    case Z_AXIS: {

    #if CORE_IS_XZ
        BABYSTEP_AXIS(X, BABYSTEP_INVERT_Z, direction);
        BABYSTEP_AXIS(Z, BABYSTEP_INVERT_Z, direction ^ (CORESIGN(1) < 0));

    #elif CORE_IS_YZ
        BABYSTEP_AXIS(Y, BABYSTEP_INVERT_Z, direction);
        BABYSTEP_AXIS(Z, BABYSTEP_INVERT_Z, direction ^ (CORESIGN(1) < 0));

    #elif DISABLED(DELTA)
        BABYSTEP_AXIS(Z, BABYSTEP_INVERT_Z, direction);

    #else // DELTA

        const bool z_direction = direction ^ BABYSTEP_INVERT_Z;

        enable_XY();
        enable_Z();

        #if MINIMUM_STEPPER_PRE_DIR_DELAY > 0
        delay_ns_precise<MINIMUM_STEPPER_PRE_DIR_DELAY>();
        #endif

        const uint8_t old_x_dir_pin = X_DIR_READ(),
                      old_y_dir_pin = Y_DIR_READ(),
                      old_z_dir_pin = Z_DIR_READ();

        X_DIR_WRITE(INVERT_X_DIR ^ z_direction);
        Y_DIR_WRITE(INVERT_Y_DIR ^ z_direction);
        Z_DIR_WRITE(INVERT_Z_DIR ^ z_direction);

        #if MINIMUM_STEPPER_POST_DIR_DELAY > 0
        delay_ns_precise<MINIMUM_STEPPER_POST_DIR_DELAY>();
        #endif

        _SAVE_START;

        X_STEP_WRITE(!INVERT_X_STEP_PIN);
        Y_STEP_WRITE(!INVERT_Y_STEP_PIN);
        Z_STEP_WRITE(!INVERT_Z_STEP_PIN);

        _PULSE_WAIT;

        X_STEP_WRITE(INVERT_X_STEP_PIN);
        Y_STEP_WRITE(INVERT_Y_STEP_PIN);
        Z_STEP_WRITE(INVERT_Z_STEP_PIN);

        // Restore direction bits
        X_DIR_WRITE(old_x_dir_pin);
        Y_DIR_WRITE(old_y_dir_pin);
        Z_DIR_WRITE(old_z_dir_pin);

    #endif

    } break;

    default:
        break;
    }
    CRITICAL_SECTION_END;
}

#endif // BABYSTEPPING

#if HAS_DRIVER(TMC2130) || HAS_DRIVER(TMC2209)
    #include "config_store/store_c_api.h"
void Stepper::microstep_mode(const uint8_t driver, const uint8_t stepping) {
    switch (driver) {
    case 0:
    #if AXIS_IS_TMC(X)
        stepperX.microsteps(stepping);
    #endif
    #if AXIS_IS_TMC(X2)
        stepperX2.microsteps(stepping);
    #endif
        break;
    case 1:
    #if AXIS_IS_TMC(Y)
        stepperY.microsteps(stepping);
    #endif
    #if AXIS_IS_TMC(Y2)
        stepperY2.microsteps(stepping);
    #endif
        break;
    case 2:
    #if AXIS_IS_TMC(Z)
        stepperZ.microsteps(stepping);
    #endif
    #if AXIS_IS_TMC(Z2)
        stepperZ2.microsteps(steping);
    #endif
    #if AXIS_IS_TMC(Z3)
        stepperZ3.microsteps(stepping);
    #endif
        break;
    case 3:
    #if AXIS_IS_TMC(E0)
        stepperE0.microsteps(stepping);
    #endif
    #if AXIS_IS_TMC(E1)
        stepperE1.microsteps(stepping);
    #endif
    #if AXIS_IS_TMC(E2)
        stepperE2.microsteps(stepping);
    #endif
    #if AXIS_IS_TMC(E3)
        stepperE3.microsteps(stepping);
    #endif
    #if AXIS_IS_TMC(E4)
        stepperE4.microsteps(stepping);
    #endif
    #if AXIS_IS_TMC(E5)
        stepperE5.microsteps(stepping);
    #endif
        break;

    default:
        SERIAL_ERROR_MSG("Axis unavailable");
        break;
    }
}
void Stepper::microstep_readings() {
    char msg[7]; // max len of message is 256
    SERIAL_ECHOPGM("Microsteps are:\n");

    #if AXIS_IS_TMC(X)
    SERIAL_ECHOPGM("X:");
    snprintf(msg, 7, "%u\n", stepperX.microsteps());
    SERIAL_ECHOPGM(msg);
    #endif
    #if AXIS_IS_TMC(X2)
    SERIAL_ECHOPGM("X2:");
    snprintf(msg, 7, "%u\n", stepperX2.microsteps());
    stepperX2.microsteps();
    SERIAL_ECHOPGM(msg);
    #endif
    #if AXIS_IS_TMC(Y)
    SERIAL_ECHOPGM("Y:");
    snprintf(msg, 7, "%u\n", stepperY.microsteps());
    SERIAL_ECHOPGM(msg);
    #endif
    #if AXIS_IS_TMC(Y2)
    SERIAL_ECHOPGM("Y2:");
    snprintf(msg, 7, "%u\n", stepperY2.microsteps());
    SERIAL_ECHOPGM(msg);
    #endif
    #if AXIS_IS_TMC(Z)
    SERIAL_ECHOPGM("Z:");
    snprintf(msg, 7, "%u\n", stepperZ.microsteps());
    SERIAL_ECHOPGM(msg);
    #endif
    #if AXIS_IS_TMC(Z2)
    SERIAL_ECHOPGM("Z2:");
    snprintf(msg, 7, "%u\n", stepperZ2.microsteps());
    SERIAL_ECHOPGM(msg);
    #endif
    #if AXIS_IS_TMC(Z3)
    SERIAL_ECHOPGM("Z3:");
    snprintf(msg, 7, "%u\n", stepperZ3.microsteps());
    SERIAL_ECHOPGM(msg);
    #endif
    #if AXIS_IS_TMC(E0)
    SERIAL_ECHOPGM("E0:");
    snprintf(msg, 7, "%u\n", stepperE0.microsteps());
    SERIAL_ECHOPGM(msg);
    #endif
    #if AXIS_IS_TMC(E1)
    SERIAL_ECHOPGM("E1:");
    snprintf(msg, 7, "%u\n", stepperE1.microsteps());
    SERIAL_ECHOPGM(msg);
    #endif
    #if AXIS_IS_TMC(E2)
    SERIAL_ECHOPGM("E2:");
    snprintf(msg, 7, "%u\n", stepperE2.microsteps());
    SERIAL_ECHOPGM(msg);
    #endif
    #if AXIS_IS_TMC(E3)
    SERIAL_ECHOPGM("E3:");
    snprintf(msg, 7, "%u\n", stepperE2.microsteps());
    SERIAL_ECHOPGM(msg);
    #endif
    #if AXIS_IS_TMC(E4)
    SERIAL_ECHOPGM("E4:");
    snprintf(msg, 7, "%u\n", stepperE3.microsteps());
    SERIAL_ECHOPGM(msg);
    #endif
    #if AXIS_IS_TMC(E5)
    SERIAL_ECHOPGM("E5:");
    snprintf(msg, 7, "%u\n", stepperE5.microsteps());
    SERIAL_ECHOPGM(msg);
    #endif
}
#endif // HAS_DRIVER(TMC2130) || HAS_DRIVER(TMC2209)
