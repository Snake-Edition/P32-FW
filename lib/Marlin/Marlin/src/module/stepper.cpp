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

#if HAS_MOTOR_CURRENT_PWM
  bool Stepper::initialized; // = false
#endif

#ifdef __AVR__
  #include "speed_lookuptable.h"
#endif

#include "endstops.h"
#include "planner.h"
#include "motion.h"

#include "temperature.h"
#include "../lcd/ultralcd.h"
#include "../core/language.h"
#include "../gcode/queue.h"
#include "../sd/cardreader.h"
#include "../Marlin.h"
#include "../HAL/shared/Delay.h"

#if MB(ALLIGATOR)
  #include "../feature/dac/dac_dac084s085.h"
#endif

#if HAS_DIGIPOTSS
  #include <SPI.h>
#endif

#if ENABLED(MIXING_EXTRUDER)
  #include "../feature/mixing.h"
#endif

#ifdef FILAMENT_RUNOUT_DISTANCE_MM
  #include "../feature/runout.h"
#endif

#if HAS_DRIVER(L6470)
  #include "../libs/L6470/L6470_Marlin.h"
#endif

// public:

#if HAS_EXTRA_ENDSTOPS || ENABLED(Z_STEPPER_AUTO_ALIGN)
  bool Stepper::separate_multi_axis = false;
#endif

#if HAS_MOTOR_CURRENT_PWM
  uint32_t Stepper::motor_current_setting[3]; // Initialized by settings.load()
#endif

// private:

uint8_t Stepper::last_direction_bits, // = 0
        Stepper::axis_did_move; // = 0

xyz_long_t Stepper::endstops_trigsteps;
xyze_long_t Stepper::count_position{0};
xyze_long_t Stepper::count_position_from_startup{0};
xyze_long_t Stepper::count_position_last_block{0};
xyze_int8_t Stepper::count_direction{0};

/**
 * Set the stepper direction of each axis
 *
 *   COREXY: X_AXIS=A_AXIS and Y_AXIS=B_AXIS
 *   COREXZ: X_AXIS=A_AXIS and Z_AXIS=C_AXIS
 *   COREYZ: Y_AXIS=B_AXIS and Z_AXIS=C_AXIS
 */
void Stepper::set_directions() {

  #if HAS_DRIVER(L6470)
    uint8_t L6470_buf[MAX_L6470 + 1];   // chip command sequence - element 0 not used
  #endif

  #if MINIMUM_STEPPER_PRE_DIR_DELAY > 0
    DELAY_NS(MINIMUM_STEPPER_PRE_DIR_DELAY);
  #endif

  #define SET_STEP_DIR(A)                       \
    if (motor_direction(_AXIS(A))) {            \
      A##_APPLY_DIR(INVERT_## A##_DIR);  \
      count_direction[_AXIS(A)] = -1;           \
    }                                           \
    else {                                      \
      A##_APPLY_DIR(!INVERT_## A##_DIR); \
      count_direction[_AXIS(A)] = 1;            \
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

  #if ENABLED(MIXING_EXTRUDER)
     // Because this is valid for the whole block we don't know
     // what e-steppers will step. Likely all. Set all.
    if (motor_direction(E_AXIS)) {
      MIXER_STEPPER_LOOP(j) REV_E_DIR(j);
      count_direction.e = -1;
    }
    else {
      MIXER_STEPPER_LOOP(j) NORM_E_DIR(j);
      count_direction.e = 1;
    }
  #else
    if (motor_direction(E_AXIS)) {
      REV_E_DIR(stepper_extruder);
      count_direction.e = -1;
    }
    else {
      NORM_E_DIR(stepper_extruder);
      count_direction.e = 1;
    }
  #endif

  #if HAS_DRIVER(L6470)

    if (L6470.spi_active) {
      L6470.spi_abort = true;                     // interrupted a SPI transfer - need to shut it down gracefully
      for (uint8_t j = 1; j <= L6470::chain[0]; j++)
        L6470_buf[j] = dSPIN_NOP;                 // fill buffer with NOOP commands
      L6470.transfer(L6470_buf, L6470::chain[0]);  // send enough NOOPs to complete any command
      L6470.transfer(L6470_buf, L6470::chain[0]);
      L6470.transfer(L6470_buf, L6470::chain[0]);
    }

    // The L6470.dir_commands[] array holds the direction command for each stepper

    //scan command array and copy matches into L6470.transfer
    for (uint8_t j = 1; j <= L6470::chain[0]; j++)
      L6470_buf[j] = L6470.dir_commands[L6470::chain[j]];

    L6470.transfer(L6470_buf, L6470::chain[0]);  // send the command stream to the drivers

  #endif

  // A small delay may be needed after changing direction
  #if MINIMUM_STEPPER_POST_DIR_DELAY > 0
    DELAY_NS(MINIMUM_STEPPER_POST_DIR_DELAY);
  #endif
}

// Return ratio of completed steps of current block (call within ISR context)
float Stepper::segment_progress() {
  const block_t *current_block = planner.get_current_processed_block();
  if (!current_block || !current_block->mstep_event_count) return NAN;

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
    case X_AXIS: return INVERT_X_DIR;
    case Y_AXIS: return INVERT_Y_DIR;
    case Z_AXIS: return INVERT_Z_DIR;
    default: return false; // other axes cannot be inverted
  }
}

#define _APPLY_STEP(AXIS) AXIS ##_STEP_WRITE
#define _INVERT_STEP_PIN(AXIS) INVERT_## AXIS ##_STEP_PIN

void Stepper::init() {

  #if MB(ALLIGATOR)
    const float motor_current[] = MOTOR_CURRENT;
    unsigned int digipot_motor = 0;
    for (uint8_t i = 0; i < 3 + EXTRUDERS; i++) {
      digipot_motor = 255 * (motor_current[i] / 2.5);
      dac084s085::setValue(i, digipot_motor);
    }
  #endif//MB(ALLIGATOR)

  // Init Microstepping Pins
  #if HAS_MICROSTEPS
    microstep_init();
  #endif

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
    if (!X_ENABLE_ON) X_ENABLE_WRITE(HIGH);
    #if EITHER(DUAL_X_CARRIAGE, X_DUAL_STEPPER_DRIVERS) && HAS_X2_ENABLE
      X2_ENABLE_INIT;
      if (!X_ENABLE_ON) X2_ENABLE_WRITE(HIGH);
    #endif
  #endif
  #if HAS_Y_ENABLE
    Y_ENABLE_INIT;
    if (!Y_ENABLE_ON) Y_ENABLE_WRITE(HIGH);
    #if ENABLED(Y_DUAL_STEPPER_DRIVERS) && HAS_Y2_ENABLE
      Y2_ENABLE_INIT;
      if (!Y_ENABLE_ON) Y2_ENABLE_WRITE(HIGH);
    #endif
  #endif
  #if HAS_Z_ENABLE
    Z_ENABLE_INIT;
    if (!Z_ENABLE_ON) Z_ENABLE_WRITE(HIGH);
    #if Z_MULTI_STEPPER_DRIVERS && HAS_Z2_ENABLE
      Z2_ENABLE_INIT;
      if (!Z_ENABLE_ON) Z2_ENABLE_WRITE(HIGH);
    #endif
    #if ENABLED(Z_TRIPLE_STEPPER_DRIVERS) && HAS_Z3_ENABLE
      Z3_ENABLE_INIT;
      if (!Z_ENABLE_ON) Z3_ENABLE_WRITE(HIGH);
    #endif
  #endif
  #if HAS_E0_ENABLE
    E0_ENABLE_INIT;
    if (!E_ENABLE_ON) E0_ENABLE_WRITE(HIGH);
  #endif
  #if HAS_E1_ENABLE
    E1_ENABLE_INIT;
    if (!E_ENABLE_ON) E1_ENABLE_WRITE(HIGH);
  #endif
  #if HAS_E2_ENABLE
    E2_ENABLE_INIT;
    if (!E_ENABLE_ON) E2_ENABLE_WRITE(HIGH);
  #endif
  #if HAS_E3_ENABLE
    E3_ENABLE_INIT;
    if (!E_ENABLE_ON) E3_ENABLE_WRITE(HIGH);
  #endif
  #if HAS_E4_ENABLE
    E4_ENABLE_INIT;
    if (!E_ENABLE_ON) E4_ENABLE_WRITE(HIGH);
  #endif
  #if HAS_E5_ENABLE
    E5_ENABLE_INIT;
    if (!E_ENABLE_ON) E5_ENABLE_WRITE(HIGH);
  #endif

  #define _STEP_INIT(AXIS) AXIS ##_STEP_INIT
  #define _WRITE_STEP(AXIS, HIGHLOW) AXIS ##_STEP_WRITE(HIGHLOW)
  #define _DISABLE(AXIS) disable_## AXIS()

  #define AXIS_INIT(AXIS, PIN) \
    _STEP_INIT(AXIS); \
    _WRITE_STEP(AXIS, _INVERT_STEP_PIN(PIN))

  #define E_AXIS_INIT(NUM) AXIS_INIT(E## NUM, E)

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

  #if DISABLED(I2S_STEPPER_STREAM)
    HAL_timer_start(STEP_TIMER_NUM, 122); // Init Stepper ISR to 122 Hz for quick starting
    wake_up();
    sei();
  #endif

  // Init direction bits for first moves
  last_direction_bits = 0
    | (INVERT_X_DIR ? _BV(X_AXIS) : 0)
    | (INVERT_Y_DIR ? _BV(Y_AXIS) : 0)
    | (INVERT_Z_DIR ? _BV(Z_AXIS) : 0);

  set_directions();

  #if HAS_DIGIPOTSS || HAS_MOTOR_CURRENT_PWM
    #if HAS_MOTOR_CURRENT_PWM
      initialized = true;
    #endif
    digipot_init();
  #endif
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

/**
 * Get a stepper's position in steps.
 */
int32_t Stepper::position(const AxisEnum axis) {
  #ifdef __AVR__
    // Protect the access to the position. Only required for AVR, as
    //  any 32bit CPU offers atomic access to 32bit variables
    const bool was_enabled = suspend();
  #endif

  const int32_t v = count_position[axis];

  #ifdef __AVR__
    // Reenable Stepper ISR
    if (was_enabled) wake_up();
  #endif
  return v;
}

int32_t Stepper::position_from_startup(const AxisEnum axis) {
    return count_position_from_startup[axis];
}

// Signal endstops were triggered - This function can be called from
// an ISR context  (Temperature, Stepper or limits ISR), so we must
// be very careful here. If the interrupt being preempted was the
// Stepper ISR (this CAN happen with the endstop limits ISR) then
// when the stepper ISR resumes, we must be very sure that the movement
// is properly canceled
void Stepper::endstop_triggered(const AxisEnum axis) {

  const bool was_enabled = suspend();
  endstops_trigsteps[axis] = (
    #if IS_CORE
      (axis == CORE_AXIS_2
        ? CORESIGN(count_position[CORE_AXIS_1] - count_position[CORE_AXIS_2])
        : count_position[CORE_AXIS_1] + count_position[CORE_AXIS_2]
      ) * 0.5f
    #else // !IS_CORE
      count_position[axis]
    #endif
  );

  // Discard the rest of the move if there is a current block
  PreciseStepping::quick_stop();

  if (was_enabled) wake_up();
}

int32_t Stepper::triggered_position(const AxisEnum axis) {
  #ifdef __AVR__
    // Protect the access to the position. Only required for AVR, as
    //  any 32bit CPU offers atomic access to 32bit variables
    const bool was_enabled = suspend();
  #endif

  const int32_t v = endstops_trigsteps[axis];

  #ifdef __AVR__
    // Reenable Stepper ISR
    if (was_enabled) wake_up();
  #endif

  return v;
}

void Stepper::report_positions() {

  #ifdef __AVR__
    // Protect the access to the position.
    const bool was_enabled = suspend();
  #endif

  const xyz_long_t pos = count_position;

  #ifdef __AVR__
    if (was_enabled) wake_up();
  #endif

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
    #define STEP_PULSE_CYCLES ((MINIMUM_STEPPER_PULSE) * CYCLES_PER_MICROSECOND)
  #else
    #define STEP_PULSE_CYCLES 0
  #endif

  #if ENABLED(DELTA)
    #define CYCLES_EATEN_BABYSTEP (2 * 15)
  #else
    #define CYCLES_EATEN_BABYSTEP 0
  #endif
  #define EXTRA_CYCLES_BABYSTEP (STEP_PULSE_CYCLES - (CYCLES_EATEN_BABYSTEP))

  #define _ENABLE(AXIS) enable_## AXIS()
  #define _READ_DIR(AXIS) AXIS ##_DIR_READ()
  #define _INVERT_DIR(AXIS) INVERT_## AXIS ##_DIR
  #define _APPLY_DIR(AXIS, INVERT) AXIS ##_APPLY_DIR(INVERT)

  #if EXTRA_CYCLES_BABYSTEP > 20
    #define _SAVE_START const hal_timer_t pulse_start = HAL_timer_get_count(PULSE_TIMER_NUM)
    #define _PULSE_WAIT while (EXTRA_CYCLES_BABYSTEP > (uint32_t)(HAL_timer_get_count(PULSE_TIMER_NUM) - pulse_start) * (PULSE_TIMER_PRESCALE)) { /* nada */ }
  #else
    #define _SAVE_START NOOP
    #if EXTRA_CYCLES_BABYSTEP > 0
      #define _PULSE_WAIT DELAY_NS(EXTRA_CYCLES_BABYSTEP * NANOSECONDS_PER_CYCLE)
    #elif ENABLED(DELTA)
      #define _PULSE_WAIT DELAY_US(2);
    #elif STEP_PULSE_CYCLES > 0
      #define _PULSE_WAIT NOOP
    #else
      #define _PULSE_WAIT DELAY_US(4);
    #endif
  #endif

  #define BABYSTEP_AXIS(AXIS, INVERT, DIR) {            \
      const uint8_t old_dir = _READ_DIR(AXIS);          \
      _ENABLE(AXIS);                                    \
      DELAY_NS(MINIMUM_STEPPER_PRE_DIR_DELAY);          \
      _APPLY_DIR(AXIS, _INVERT_DIR(AXIS)^DIR^INVERT);   \
      DELAY_NS(MINIMUM_STEPPER_POST_DIR_DELAY);         \
      _SAVE_START;                                      \
      _APPLY_STEP(AXIS)(!_INVERT_STEP_PIN(AXIS));       \
      _PULSE_WAIT;                                      \
      _APPLY_STEP(AXIS)(_INVERT_STEP_PIN(AXIS));        \
      _APPLY_DIR(AXIS, old_dir);                        \
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
            BABYSTEP_AXIS(Y, false, direction^(CORESIGN(1)<0));
          #elif CORE_IS_YZ
            BABYSTEP_AXIS(Y, false, direction);
            BABYSTEP_AXIS(Z, false, direction^(CORESIGN(1)<0));
          #else
            BABYSTEP_AXIS(Y, false, direction);
          #endif
          break;

      #endif

      case Z_AXIS: {

        #if CORE_IS_XZ
          BABYSTEP_AXIS(X, BABYSTEP_INVERT_Z, direction);
          BABYSTEP_AXIS(Z, BABYSTEP_INVERT_Z, direction^(CORESIGN(1)<0));

        #elif CORE_IS_YZ
          BABYSTEP_AXIS(Y, BABYSTEP_INVERT_Z, direction);
          BABYSTEP_AXIS(Z, BABYSTEP_INVERT_Z, direction^(CORESIGN(1)<0));

        #elif DISABLED(DELTA)
          BABYSTEP_AXIS(Z, BABYSTEP_INVERT_Z, direction);

        #else // DELTA

          const bool z_direction = direction ^ BABYSTEP_INVERT_Z;

          enable_XY();
          enable_Z();

          #if MINIMUM_STEPPER_PRE_DIR_DELAY > 0
            DELAY_NS(MINIMUM_STEPPER_PRE_DIR_DELAY);
          #endif

          const uint8_t old_x_dir_pin = X_DIR_READ(),
                        old_y_dir_pin = Y_DIR_READ(),
                        old_z_dir_pin = Z_DIR_READ();

          X_DIR_WRITE(INVERT_X_DIR ^ z_direction);
          Y_DIR_WRITE(INVERT_Y_DIR ^ z_direction);
          Z_DIR_WRITE(INVERT_Z_DIR ^ z_direction);

          #if MINIMUM_STEPPER_POST_DIR_DELAY > 0
            DELAY_NS(MINIMUM_STEPPER_POST_DIR_DELAY);
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

      default: break;
    }
    CRITICAL_SECTION_END;
  }

#endif // BABYSTEPPING

/**
 * Software-controlled Stepper Motor Current
 */

#if HAS_DIGIPOTSS

  // From Arduino DigitalPotControl example
  void Stepper::digitalPotWrite(const int16_t address, const int16_t value) {
    WRITE(DIGIPOTSS_PIN, LOW);  // Take the SS pin low to select the chip
    SPI.transfer(address);      // Send the address and value via SPI
    SPI.transfer(value);
    WRITE(DIGIPOTSS_PIN, HIGH); // Take the SS pin high to de-select the chip
    //delay(10);
  }

#endif // HAS_DIGIPOTSS

#if HAS_MOTOR_CURRENT_PWM

  void Stepper::refresh_motor_power() {
    if (!initialized) return;
    LOOP_L_N(i, COUNT(motor_current_setting)) {
      switch (i) {
        #if ANY_PIN(MOTOR_CURRENT_PWM_XY, MOTOR_CURRENT_PWM_X, MOTOR_CURRENT_PWM_Y)
          case 0:
        #endif
        #if PIN_EXISTS(MOTOR_CURRENT_PWM_Z)
          case 1:
        #endif
        #if ANY_PIN(MOTOR_CURRENT_PWM_E, MOTOR_CURRENT_PWM_E0, MOTOR_CURRENT_PWM_E1)
          case 2:
        #endif
            digipot_current(i, motor_current_setting[i]);
        default: break;
      }
    }
  }

#endif // HAS_MOTOR_CURRENT_PWM

#if !MB(PRINTRBOARD_G2)

  #if HAS_DIGIPOTSS || HAS_MOTOR_CURRENT_PWM

    void Stepper::digipot_current(const uint8_t driver, const int16_t current) {

      #if HAS_DIGIPOTSS

        const uint8_t digipot_ch[] = DIGIPOT_CHANNELS;
        digitalPotWrite(digipot_ch[driver], current);

      #elif HAS_MOTOR_CURRENT_PWM

        if (!initialized) return;

        if (WITHIN(driver, 0, COUNT(motor_current_setting) - 1))
          motor_current_setting[driver] = current; // update motor_current_setting

        #define _WRITE_CURRENT_PWM(P) analogWrite(pin_t(MOTOR_CURRENT_PWM_## P ##_PIN), 255L * current / (MOTOR_CURRENT_PWM_RANGE))
        switch (driver) {
          case 0:
            #if PIN_EXISTS(MOTOR_CURRENT_PWM_X)
              _WRITE_CURRENT_PWM(X);
            #endif
            #if PIN_EXISTS(MOTOR_CURRENT_PWM_Y)
              _WRITE_CURRENT_PWM(Y);
            #endif
            #if PIN_EXISTS(MOTOR_CURRENT_PWM_XY)
              _WRITE_CURRENT_PWM(XY);
            #endif
            break;
          case 1:
            #if PIN_EXISTS(MOTOR_CURRENT_PWM_Z)
              _WRITE_CURRENT_PWM(Z);
            #endif
            break;
          case 2:
            #if PIN_EXISTS(MOTOR_CURRENT_PWM_E)
              _WRITE_CURRENT_PWM(E);
            #endif
            #if PIN_EXISTS(MOTOR_CURRENT_PWM_E0)
              _WRITE_CURRENT_PWM(E0);
            #endif
            #if PIN_EXISTS(MOTOR_CURRENT_PWM_E1)
              _WRITE_CURRENT_PWM(E1);
            #endif
            break;
        }
      #endif
    }

    void Stepper::digipot_init() {

      #if HAS_DIGIPOTSS

        static const uint8_t digipot_motor_current[] = DIGIPOT_MOTOR_CURRENT;

        SPI.begin();
        SET_OUTPUT(DIGIPOTSS_PIN);

        for (uint8_t i = 0; i < COUNT(digipot_motor_current); i++) {
          //digitalPotWrite(digipot_ch[i], digipot_motor_current[i]);
          digipot_current(i, digipot_motor_current[i]);
        }

      #elif HAS_MOTOR_CURRENT_PWM

        #if PIN_EXISTS(MOTOR_CURRENT_PWM_X)
          SET_PWM(MOTOR_CURRENT_PWM_X_PIN);
        #endif
        #if PIN_EXISTS(MOTOR_CURRENT_PWM_Y)
          SET_PWM(MOTOR_CURRENT_PWM_Y_PIN);
        #endif
        #if PIN_EXISTS(MOTOR_CURRENT_PWM_XY)
          SET_PWM(MOTOR_CURRENT_PWM_XY_PIN);
        #endif
        #if PIN_EXISTS(MOTOR_CURRENT_PWM_Z)
          SET_PWM(MOTOR_CURRENT_PWM_Z_PIN);
        #endif
        #if PIN_EXISTS(MOTOR_CURRENT_PWM_E)
          SET_PWM(MOTOR_CURRENT_PWM_E_PIN);
        #endif
        #if PIN_EXISTS(MOTOR_CURRENT_PWM_E0)
          SET_PWM(MOTOR_CURRENT_PWM_E0_PIN);
        #endif
        #if PIN_EXISTS(MOTOR_CURRENT_PWM_E1)
          SET_PWM(MOTOR_CURRENT_PWM_E1_PIN);
        #endif

        refresh_motor_power();

        // Set Timer5 to 31khz so the PWM of the motor power is as constant as possible. (removes a buzzing noise)
        #ifdef __AVR__
          SET_CS5(PRESCALER_1);
        #endif
      #endif
    }

  #endif

#else // PRINTRBOARD_G2

  #include HAL_PATH(../HAL, fastio/G2_PWM.h)

#endif

#if HAS_MICROSTEPS

  /**
   * Software-controlled Microstepping
   */

  void Stepper::microstep_init() {
    #if HAS_X_MICROSTEPS
      SET_OUTPUT(X_MS1_PIN);
      SET_OUTPUT(X_MS2_PIN);
      #if PIN_EXISTS(X_MS3)
        SET_OUTPUT(X_MS3_PIN);
      #endif
    #endif
    #if HAS_X2_MICROSTEPS
      SET_OUTPUT(X2_MS1_PIN);
      SET_OUTPUT(X2_MS2_PIN);
      #if PIN_EXISTS(X2_MS3)
        SET_OUTPUT(X2_MS3_PIN);
      #endif
    #endif
    #if HAS_Y_MICROSTEPS
      SET_OUTPUT(Y_MS1_PIN);
      SET_OUTPUT(Y_MS2_PIN);
      #if PIN_EXISTS(Y_MS3)
        SET_OUTPUT(Y_MS3_PIN);
      #endif
    #endif
    #if HAS_Y2_MICROSTEPS
      SET_OUTPUT(Y2_MS1_PIN);
      SET_OUTPUT(Y2_MS2_PIN);
      #if PIN_EXISTS(Y2_MS3)
        SET_OUTPUT(Y2_MS3_PIN);
      #endif
    #endif
    #if HAS_Z_MICROSTEPS
      SET_OUTPUT(Z_MS1_PIN);
      SET_OUTPUT(Z_MS2_PIN);
      #if PIN_EXISTS(Z_MS3)
        SET_OUTPUT(Z_MS3_PIN);
      #endif
    #endif
    #if HAS_Z2_MICROSTEPS
      SET_OUTPUT(Z2_MS1_PIN);
      SET_OUTPUT(Z2_MS2_PIN);
      #if PIN_EXISTS(Z2_MS3)
        SET_OUTPUT(Z2_MS3_PIN);
      #endif
    #endif
    #if HAS_Z3_MICROSTEPS
      SET_OUTPUT(Z3_MS1_PIN);
      SET_OUTPUT(Z3_MS2_PIN);
      #if PIN_EXISTS(Z3_MS3)
        SET_OUTPUT(Z3_MS3_PIN);
      #endif
    #endif
    #if HAS_E0_MICROSTEPS
      SET_OUTPUT(E0_MS1_PIN);
      SET_OUTPUT(E0_MS2_PIN);
      #if PIN_EXISTS(E0_MS3)
        SET_OUTPUT(E0_MS3_PIN);
      #endif
    #endif
    #if HAS_E1_MICROSTEPS
      SET_OUTPUT(E1_MS1_PIN);
      SET_OUTPUT(E1_MS2_PIN);
      #if PIN_EXISTS(E1_MS3)
        SET_OUTPUT(E1_MS3_PIN);
      #endif
    #endif
    #if HAS_E2_MICROSTEPS
      SET_OUTPUT(E2_MS1_PIN);
      SET_OUTPUT(E2_MS2_PIN);
      #if PIN_EXISTS(E2_MS3)
        SET_OUTPUT(E2_MS3_PIN);
      #endif
    #endif
    #if HAS_E3_MICROSTEPS
      SET_OUTPUT(E3_MS1_PIN);
      SET_OUTPUT(E3_MS2_PIN);
      #if PIN_EXISTS(E3_MS3)
        SET_OUTPUT(E3_MS3_PIN);
      #endif
    #endif
    #if HAS_E4_MICROSTEPS
      SET_OUTPUT(E4_MS1_PIN);
      SET_OUTPUT(E4_MS2_PIN);
      #if PIN_EXISTS(E4_MS3)
        SET_OUTPUT(E4_MS3_PIN);
      #endif
    #endif
    #if HAS_E5_MICROSTEPS
      SET_OUTPUT(E5_MS1_PIN);
      SET_OUTPUT(E5_MS2_PIN);
      #if PIN_EXISTS(E5_MS3)
        SET_OUTPUT(E5_MS3_PIN);
      #endif
    #endif

    static const uint8_t microstep_modes[] = MICROSTEP_MODES;
    for (uint16_t i = 0; i < COUNT(microstep_modes); i++)
      microstep_mode(i, microstep_modes[i]);
  }

  void Stepper::microstep_ms(const uint8_t driver, const int8_t ms1, const int8_t ms2, const int8_t ms3) {
    if (ms1 >= 0) switch (driver) {
      #if HAS_X_MICROSTEPS || HAS_X2_MICROSTEPS
        case 0:
          #if HAS_X_MICROSTEPS
            WRITE(X_MS1_PIN, ms1);
          #endif
          #if HAS_X2_MICROSTEPS
            WRITE(X2_MS1_PIN, ms1);
          #endif
          break;
      #endif
      #if HAS_Y_MICROSTEPS || HAS_Y2_MICROSTEPS
        case 1:
          #if HAS_Y_MICROSTEPS
            WRITE(Y_MS1_PIN, ms1);
          #endif
          #if HAS_Y2_MICROSTEPS
            WRITE(Y2_MS1_PIN, ms1);
          #endif
          break;
      #endif
      #if HAS_Z_MICROSTEPS || HAS_Z2_MICROSTEPS || HAS_Z3_MICROSTEPS
        case 2:
          #if HAS_Z_MICROSTEPS
            WRITE(Z_MS1_PIN, ms1);
          #endif
          #if HAS_Z2_MICROSTEPS
            WRITE(Z2_MS1_PIN, ms1);
          #endif
          #if HAS_Z3_MICROSTEPS
            WRITE(Z3_MS1_PIN, ms1);
          #endif
          break;
      #endif
      #if HAS_E0_MICROSTEPS
        case 3: WRITE(E0_MS1_PIN, ms1); break;
      #endif
      #if HAS_E1_MICROSTEPS
        case 4: WRITE(E1_MS1_PIN, ms1); break;
      #endif
      #if HAS_E2_MICROSTEPS
        case 5: WRITE(E2_MS1_PIN, ms1); break;
      #endif
      #if HAS_E3_MICROSTEPS
        case 6: WRITE(E3_MS1_PIN, ms1); break;
      #endif
      #if HAS_E4_MICROSTEPS
        case 7: WRITE(E4_MS1_PIN, ms1); break;
      #endif
      #if HAS_E5_MICROSTEPS
        case 8: WRITE(E5_MS1_PIN, ms1); break;
      #endif
    }
    if (ms2 >= 0) switch (driver) {
      #if HAS_X_MICROSTEPS || HAS_X2_MICROSTEPS
        case 0:
          #if HAS_X_MICROSTEPS
            WRITE(X_MS2_PIN, ms2);
          #endif
          #if HAS_X2_MICROSTEPS
            WRITE(X2_MS2_PIN, ms2);
          #endif
          break;
      #endif
      #if HAS_Y_MICROSTEPS || HAS_Y2_MICROSTEPS
        case 1:
          #if HAS_Y_MICROSTEPS
            WRITE(Y_MS2_PIN, ms2);
          #endif
          #if HAS_Y2_MICROSTEPS
            WRITE(Y2_MS2_PIN, ms2);
          #endif
          break;
      #endif
      #if HAS_Z_MICROSTEPS || HAS_Z2_MICROSTEPS || HAS_Z3_MICROSTEPS
        case 2:
          #if HAS_Z_MICROSTEPS
            WRITE(Z_MS2_PIN, ms2);
          #endif
          #if HAS_Z2_MICROSTEPS
            WRITE(Z2_MS2_PIN, ms2);
          #endif
          #if HAS_Z3_MICROSTEPS
            WRITE(Z3_MS2_PIN, ms2);
          #endif
          break;
      #endif
      #if HAS_E0_MICROSTEPS
        case 3: WRITE(E0_MS2_PIN, ms2); break;
      #endif
      #if HAS_E1_MICROSTEPS
        case 4: WRITE(E1_MS2_PIN, ms2); break;
      #endif
      #if HAS_E2_MICROSTEPS
        case 5: WRITE(E2_MS2_PIN, ms2); break;
      #endif
      #if HAS_E3_MICROSTEPS
        case 6: WRITE(E3_MS2_PIN, ms2); break;
      #endif
      #if HAS_E4_MICROSTEPS
        case 7: WRITE(E4_MS2_PIN, ms2); break;
      #endif
      #if HAS_E5_MICROSTEPS
        case 8: WRITE(E5_MS2_PIN, ms2); break;
      #endif
    }
    if (ms3 >= 0) switch (driver) {
      #if HAS_X_MICROSTEPS || HAS_X2_MICROSTEPS
        case 0:
          #if HAS_X_MICROSTEPS && PIN_EXISTS(X_MS3)
            WRITE(X_MS3_PIN, ms3);
          #endif
          #if HAS_X2_MICROSTEPS && PIN_EXISTS(X2_MS3)
            WRITE(X2_MS3_PIN, ms3);
          #endif
          break;
      #endif
      #if HAS_Y_MICROSTEPS || HAS_Y2_MICROSTEPS
        case 1:
          #if HAS_Y_MICROSTEPS && PIN_EXISTS(Y_MS3)
            WRITE(Y_MS3_PIN, ms3);
          #endif
          #if HAS_Y2_MICROSTEPS && PIN_EXISTS(Y2_MS3)
            WRITE(Y2_MS3_PIN, ms3);
          #endif
          break;
      #endif
      #if HAS_Z_MICROSTEPS || HAS_Z2_MICROSTEPS || HAS_Z3_MICROSTEPS
        case 2:
          #if HAS_Z_MICROSTEPS && PIN_EXISTS(Z_MS3)
            WRITE(Z_MS3_PIN, ms3);
          #endif
          #if HAS_Z2_MICROSTEPS && PIN_EXISTS(Z2_MS3)
            WRITE(Z2_MS3_PIN, ms3);
          #endif
          #if HAS_Z3_MICROSTEPS && PIN_EXISTS(Z3_MS3)
            WRITE(Z3_MS3_PIN, ms3);
          #endif
          break;
      #endif
      #if HAS_E0_MICROSTEPS && PIN_EXISTS(E0_MS3)
        case 3: WRITE(E0_MS3_PIN, ms3); break;
      #endif
      #if HAS_E1_MICROSTEPS && PIN_EXISTS(E1_MS3)
        case 4: WRITE(E1_MS3_PIN, ms3); break;
      #endif
      #if HAS_E2_MICROSTEPS && PIN_EXISTS(E2_MS3)
        case 5: WRITE(E2_MS3_PIN, ms3); break;
      #endif
      #if HAS_E3_MICROSTEPS && PIN_EXISTS(E3_MS3)
        case 6: WRITE(E3_MS3_PIN, ms3); break;
      #endif
      #if HAS_E4_MICROSTEPS && PIN_EXISTS(E4_MS3)
        case 7: WRITE(E4_MS3_PIN, ms3); break;
      #endif
      #if HAS_E5_MICROSTEPS && PIN_EXISTS(E5_MS3)
        case 8: WRITE(E5_MS3_PIN, ms3); break;
      #endif
    }
  }

  void Stepper::microstep_mode(const uint8_t driver, const uint8_t stepping_mode) {
    switch (stepping_mode) {
      #if HAS_MICROSTEP1
        case 1: microstep_ms(driver, MICROSTEP1); break;
      #endif
      #if HAS_MICROSTEP2
        case 2: microstep_ms(driver, MICROSTEP2); break;
      #endif
      #if HAS_MICROSTEP4
        case 4: microstep_ms(driver, MICROSTEP4); break;
      #endif
      #if HAS_MICROSTEP8
        case 8: microstep_ms(driver, MICROSTEP8); break;
      #endif
      #if HAS_MICROSTEP16
        case 16: microstep_ms(driver, MICROSTEP16); break;
      #endif
      #if HAS_MICROSTEP32
        case 32: microstep_ms(driver, MICROSTEP32); break;
      #endif
      #if HAS_MICROSTEP64
        case 64: microstep_ms(driver, MICROSTEP64); break;
      #endif
      #if HAS_MICROSTEP128
        case 128: microstep_ms(driver, MICROSTEP128); break;
      #endif

      default: SERIAL_ERROR_MSG("Microsteps unavailable"); break;
    }
  }

  void Stepper::microstep_readings() {
    SERIAL_ECHOPGM("MS1,MS2,MS3 Pins\nX: ");
    #if HAS_X_MICROSTEPS
      SERIAL_CHAR('0' + READ(X_MS1_PIN));
      SERIAL_CHAR('0' + READ(X_MS2_PIN));
      #if PIN_EXISTS(X_MS3)
        SERIAL_ECHOLN((int)READ(X_MS3_PIN));
      #endif
    #endif
    #if HAS_Y_MICROSTEPS
      SERIAL_ECHOPGM("Y: ");
      SERIAL_CHAR('0' + READ(Y_MS1_PIN));
      SERIAL_CHAR('0' + READ(Y_MS2_PIN));
      #if PIN_EXISTS(Y_MS3)
        SERIAL_ECHOLN((int)READ(Y_MS3_PIN));
      #endif
    #endif
    #if HAS_Z_MICROSTEPS
      SERIAL_ECHOPGM("Z: ");
      SERIAL_CHAR('0' + READ(Z_MS1_PIN));
      SERIAL_CHAR('0' + READ(Z_MS2_PIN));
      #if PIN_EXISTS(Z_MS3)
        SERIAL_ECHOLN((int)READ(Z_MS3_PIN));
      #endif
    #endif
    #if HAS_E0_MICROSTEPS
      SERIAL_ECHOPGM("E0: ");
      SERIAL_CHAR('0' + READ(E0_MS1_PIN));
      SERIAL_CHAR('0' + READ(E0_MS2_PIN));
      #if PIN_EXISTS(E0_MS3)
        SERIAL_ECHOLN((int)READ(E0_MS3_PIN));
      #endif
    #endif
    #if HAS_E1_MICROSTEPS
      SERIAL_ECHOPGM("E1: ");
      SERIAL_CHAR('0' + READ(E1_MS1_PIN));
      SERIAL_CHAR('0' + READ(E1_MS2_PIN));
      #if PIN_EXISTS(E1_MS3)
        SERIAL_ECHOLN((int)READ(E1_MS3_PIN));
      #endif
    #endif
    #if HAS_E2_MICROSTEPS
      SERIAL_ECHOPGM("E2: ");
      SERIAL_CHAR('0' + READ(E2_MS1_PIN));
      SERIAL_CHAR('0' + READ(E2_MS2_PIN));
      #if PIN_EXISTS(E2_MS3)
        SERIAL_ECHOLN((int)READ(E2_MS3_PIN));
      #endif
    #endif
    #if HAS_E3_MICROSTEPS
      SERIAL_ECHOPGM("E3: ");
      SERIAL_CHAR('0' + READ(E3_MS1_PIN));
      SERIAL_CHAR('0' + READ(E3_MS2_PIN));
      #if PIN_EXISTS(E3_MS3)
        SERIAL_ECHOLN((int)READ(E3_MS3_PIN));
      #endif
    #endif
    #if HAS_E4_MICROSTEPS
      SERIAL_ECHOPGM("E4: ");
      SERIAL_CHAR('0' + READ(E4_MS1_PIN));
      SERIAL_CHAR('0' + READ(E4_MS2_PIN));
      #if PIN_EXISTS(E4_MS3)
        SERIAL_ECHOLN((int)READ(E4_MS3_PIN));
      #endif
    #endif
    #if HAS_E5_MICROSTEPS
      SERIAL_ECHOPGM("E5: ");
      SERIAL_CHAR('0' + READ(E5_MS1_PIN));
      SERIAL_ECHOLN((int)READ(E5_MS2_PIN));
      #if PIN_EXISTS(E5_MS3)
        SERIAL_ECHOLN((int)READ(E5_MS3_PIN));
      #endif
    #endif
  }

#endif // HAS_MICROSTEPS
#if HAS_DRIVER(TMC2130) || HAS_DRIVER(TMC2209)
#include "config_store/store_c_api.h"
void Stepper::microstep_mode(const uint8_t driver, const uint8_t stepping){
    switch(driver){
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

      default: SERIAL_ERROR_MSG("Axis unavailable"); break;
    }

  }
void Stepper::microstep_readings(){
  char msg[7]; //max len of message is 256
  SERIAL_ECHOPGM("Microsteps are:\n");

    #if AXIS_IS_TMC(X)
      SERIAL_ECHOPGM("X:");
      snprintf(msg,7,"%u\n",stepperX.microsteps());
      SERIAL_ECHOPGM(msg);
    #endif
    #if AXIS_IS_TMC(X2)
      SERIAL_ECHOPGM("X2:");
      snprintf(msg,7,"%u\n",stepperX2.microsteps());
      stepperX2.microsteps();
SERIAL_ECHOPGM(msg);
#endif
    #if AXIS_IS_TMC(Y)
      SERIAL_ECHOPGM("Y:");
      snprintf(msg,7,"%u\n",stepperY.microsteps());
  SERIAL_ECHOPGM(msg);
#endif
    #if AXIS_IS_TMC(Y2)
      SERIAL_ECHOPGM("Y2:");
      snprintf(msg,7,"%u\n",stepperY2.microsteps());
SERIAL_ECHOPGM(msg);
#endif
    #if AXIS_IS_TMC(Z)
      SERIAL_ECHOPGM("Z:");
      snprintf(msg,7,"%u\n",stepperZ.microsteps());
  SERIAL_ECHOPGM(msg);
#endif
    #if AXIS_IS_TMC(Z2)
      SERIAL_ECHOPGM("Z2:");
      snprintf(msg,7,"%u\n",stepperZ2.microsteps());
SERIAL_ECHOPGM(msg);
#endif
    #if AXIS_IS_TMC(Z3)
      SERIAL_ECHOPGM("Z3:");
      snprintf(msg,7,"%u\n",stepperZ3.microsteps());
SERIAL_ECHOPGM(msg);
#endif
    #if AXIS_IS_TMC(E0)
      SERIAL_ECHOPGM("E0:");
      snprintf(msg,7,"%u\n",stepperE0.microsteps());
  SERIAL_ECHOPGM(msg);
#endif
    #if AXIS_IS_TMC(E1)
      SERIAL_ECHOPGM("E1:");
      snprintf(msg,7,"%u\n",stepperE1.microsteps());
SERIAL_ECHOPGM(msg);
#endif
    #if AXIS_IS_TMC(E2)
      SERIAL_ECHOPGM("E2:");
      snprintf(msg,7,"%u\n",stepperE2.microsteps());
SERIAL_ECHOPGM(msg);
#endif
    #if AXIS_IS_TMC(E3)
      SERIAL_ECHOPGM("E3:");
      snprintf(msg,7,"%u\n",stepperE2.microsteps());
SERIAL_ECHOPGM(msg);
#endif
    #if AXIS_IS_TMC(E4)
      SERIAL_ECHOPGM("E4:");
      snprintf(msg,7,"%u\n",stepperE3.microsteps());
SERIAL_ECHOPGM(msg);
#endif
    #if AXIS_IS_TMC(E5)
      SERIAL_ECHOPGM("E5:");
      snprintf(msg,7,"%u\n",stepperE5.microsteps());
SERIAL_ECHOPGM(msg);
#endif
}
#endif // HAS_DRIVER(TMC2130) || HAS_DRIVER(TMC2209)
