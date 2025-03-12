/**
 * This is abstraction wrapper around lower-level axes.
 * It allows Marlin to use a consistent interface even if a driver is
 * missing/disabled.
 */

#pragma once

#include "../inc/MarlinConfig.h"
#include <bsod.h>
#include <optional>

#if USE_SENSORLESS
/**
 * Disable creash detection
 * @param axis Physical axis
 * @param restore_stealth will set stealth mode (quiet mode)
 */
bool enable_crash_detection(AxisEnum axis);

/**
 * Disable creash detection
 * @param axis Physical axis
 * @param restore_stealth will set stealth mode (quiet mode)
 */
void disable_crash_detection(AxisEnum axis, bool restore_stealth);
#endif

// Track enabled status of stealthChop and only re-enable where applicable
struct sensorless_t {
    bool x, y, z, x2, y2, z2, z3;
};

/// Periodic health check of drivers
void monitor_motor_drivers();
/// Periodic service of motor drivers
void motor_driver_loop();
/// Initialize drivers, including necessary pin setup
void motor_driver_init();
/// Check connection status to the drivers
void motor_driver_check_connections();
/// Check of motor coils, mainly for selftest purposes
bool motor_check_coils(uint8_t axis);
/// Setup an (idle) stepper which is about to move
void motor_prepare_move_xy();
void motor_prepare_move_z();
void motor_prepare_move_e();

/// Motor driver refresh/reset
void restore_stepper_drivers(); // Called by PSU_ON
void reset_stepper_drivers(); // Called by settings.load / settings.reset

/// get/set microsteps, when new_microsteps is not provided, it just returns current value
uint16_t stepper_microsteps(AxisEnum axis, std::optional<uint16_t> new_microsteps = std::nullopt);
/// returns nstep position inside current qstep (0..1023)
uint16_t stepper_mscnt(const AxisEnum axis);
/// Wait for motor standstill on multiple axis
bool stepper_wait_for_standstill(uint8_t axis_mask, millis_t max_delay = 150);

/// Exclusive locking to driver's serial
void motor_serial_lock_init();
bool motor_serial_lock_acquire();
void motor_serial_lock_release();
bool motor_serial_lock_acquire_isr();
void motor_serial_lock_release_isr();
bool motor_serial_lock_held_by_isr();
void motor_serial_lock_let_isr_run();
void motor_serial_lock_mark_isr_starved();
void motor_serial_lock_clear_isr_starved();
bool motor_serial_lock_held();
bool motor_serial_lock_requested_by_task();
