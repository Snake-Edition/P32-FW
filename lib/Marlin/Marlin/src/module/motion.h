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
 * motion.h
 *
 * High-level motion commands to feed the planner
 * Some of these methods may migrate to the planner class.
 */

#include "../inc/MarlinConfig.h"

#include <inplace_function.hpp>
#include <array>
#include <span>

#if HAS_BED_PROBE
  #include "probe.h"
#endif
#include <option/has_wastebin.h>

// Axis homed and known-position states
static constexpr uint8_t xyz_bits = _BV(X_AXIS) | _BV(Y_AXIS) | _BV(Z_AXIS);

struct MoveHints {
  bool is_printing_move = false;      // The move is a printing move and should possibly count into max printed Z
};

enum class AxisHomeLevel : uint8_t {
  /// The axis it not homed at all, we could be anywhere
  not_homed,

  /// The axis is homed imprecisely (say +- 1mm). Good enough for some operations, not good enough for printing
  imprecise,

  /// The axis is homed as precisely as the printer allows
  full
};

struct AxesHomeLevel : public std::array<AxisHomeLevel, 3> {

public:
  // Inherit parent constructors and assign operators
  using array::array;
  using array::operator=;
  
  AxesHomeLevel(const array &data) : array(data) {}

  static constexpr array no_axes_homed{AxisHomeLevel::not_homed, AxisHomeLevel::not_homed, AxisHomeLevel::not_homed};

  /// \returns whether a single axis is homed to the required level
  constexpr bool is_homed(AxisEnum axis, AxisHomeLevel required_level) const {
    return at(std::to_underlying(axis)) >= required_level;
  }

  /// \returns whether all axes in the list are homed to the required level
  constexpr inline bool is_homed(std::span<const AxisEnum> axes, AxisHomeLevel required_level) const {
    for(auto axis : axes) {
      if(!is_homed(axis, required_level)) {
        return false;
      }
    }
    return true;
  }

  /// \returns whether all axes in the list are homed to the required level
  constexpr inline bool is_homed(std::initializer_list<AxisEnum> axes, AxisHomeLevel required_level) const {
    return is_homed(std::span(axes), required_level);
  }

  /// \returns whether all axes are homed to a required level
  constexpr bool is_homed(AxisHomeLevel required_level) const {
    return is_homed({X_AXIS, Y_AXIS, Z_AXIS}, required_level);
  }

};

/// To what degree are the individual axes homed
extern AxesHomeLevel axes_home_level;

inline bool all_axes_homed(AxisHomeLevel required_level = AxisHomeLevel::imprecise) { return axes_home_level.is_homed(required_level); }
inline bool all_axes_known(AxisHomeLevel required_level = AxisHomeLevel::imprecise) { return axes_home_level.is_homed(required_level); }

inline void set_all_unhomed() { axes_home_level = AxesHomeLevel::no_axes_homed; }

FORCE_INLINE bool homing_needed() {
  return !(
    #if ENABLED(HOME_AFTER_DEACTIVATE)
      all_axes_known()
    #else
      all_axes_homed()
    #endif
  );
}

// Error margin to work around float imprecision
constexpr float slop = 0.0001;

extern bool relative_mode;

extern xyze_pos_t current_position,  // High-level current tool position
                  destination;       // Destination for a move

// Scratch space for a cartesian result
extern xyz_pos_t cartes;

#if defined(XY_PROBE_SPEED_INITIAL)
  #define XY_PROBE_FEEDRATE_MM_S MMM_TO_MMS(XY_PROBE_SPEED_INITIAL)
#else
  #define XY_PROBE_FEEDRATE_MM_S PLANNER_XY_FEEDRATE()
#endif

#if ENABLED(Z_SAFE_HOMING)
  constexpr xy_float_t safe_homing_xy = { Z_SAFE_HOMING_X_POINT, Z_SAFE_HOMING_Y_POINT };
#endif

/**
 * Feed rates are often configured with mm/m
 * but the planner and stepper like mm/s units.
 */
extern const feedRate_t homing_feedrate_mm_s[XYZ];
FORCE_INLINE feedRate_t homing_feedrate(const AxisEnum a) { return pgm_read_float(&homing_feedrate_mm_s[a]); }
feedRate_t get_homing_bump_feedrate(const AxisEnum axis);

extern feedRate_t feedrate_mm_s;

extern float homing_bump_divisor[];

/**
 * Feedrate scaling is applied to all G0/G1, G2/G3, and G5 moves
 */
extern int16_t feedrate_percentage;
#define MMS_SCALED(V) ((V) * 0.01f * feedrate_percentage)

// The active extruder (tool). Set with T<extruder> command.
#if EXTRUDERS > 1
  extern uint8_t active_extruder;
#else
  constexpr uint8_t active_extruder = 0;
#endif

/**
 * Gets hotend index associated with a given extruder index.
 */
 inline uint8_t hotend_from_extruder([[maybe_unused]] const uint8_t e) {
  #if HOTENDS > 1
    return e;
  #else
    return 0;
  #endif
}

FORCE_INLINE float pgm_read_any(const float *p) { return pgm_read_float(p); }
FORCE_INLINE signed char pgm_read_any(const signed char *p) { return pgm_read_byte(p); }

#define XYZ_DEFS(T, NAME, OPT) \
  extern const XYZval<T> NAME##_P; \
  FORCE_INLINE T NAME(AxisEnum axis) { return pgm_read_any(&NAME##_P[axis]); }

XYZ_DEFS(float, base_min_pos,   MIN_POS);
XYZ_DEFS(float, base_max_pos,   MAX_POS);
XYZ_DEFS(float, base_home_pos,  HOME_POS);
XYZ_DEFS(float, max_length,     MAX_LENGTH);
XYZ_DEFS(float, home_bump_mm,   HOME_BUMP_MM);
XYZ_DEFS(signed char, home_dir, HOME_DIR);

#if HAS_WORKSPACE_OFFSET
  void update_workspace_offset(const AxisEnum axis);
#else
  #define update_workspace_offset(x) NOOP
#endif

#if HAS_HOTEND_OFFSET
  extern xyz_pos_t hotend_offset[HOTENDS];
  extern xyz_pos_t hotend_currently_applied_offset; // Difference to position without hotend offset. Used for tool park/pickup
  void reset_hotend_offsets();
#elif HOTENDS
  constexpr xyz_pos_t hotend_offset[HOTENDS] { };
#else
  constexpr xyz_pos_t hotend_offset[1]  {  };
#endif

typedef struct { xyz_pos_t min, max; } axis_limits_t;
#if HAS_SOFTWARE_ENDSTOPS
  extern bool soft_endstops_enabled;
  extern axis_limits_t soft_endstop;
  void apply_motion_limits(xyz_pos_t &target);
  void update_software_endstops(const AxisEnum axis
    #if HAS_HOTEND_OFFSET
      , const uint8_t old_tool_index=0, const uint8_t new_tool_index=0
    #endif
  );
  #define SET_SOFT_ENDSTOP_LOOSE(loose) NOOP

#else // !HAS_SOFTWARE_ENDSTOPS

  constexpr bool soft_endstops_enabled = false;
  //constexpr axis_limits_t soft_endstop = {
  //  { X_MIN_POS, Y_MIN_POS, Z_MIN_POS },
  //  { X_MAX_POS, Y_MAX_POS, Z_MAX_POS } };
  #define apply_motion_limits(V)    NOOP
  #define update_software_endstops(...) NOOP
  #define SET_SOFT_ENDSTOP_LOOSE(V)     NOOP

#endif // !HAS_SOFTWARE_ENDSTOPS

void report_current_position();

void get_cartesian_from_steppers();
void set_current_from_steppers_for_axis(const AxisEnum axis);
void set_current_from_steppers();

/**
 * sync_plan_position
 *
 * Set the planner/stepper positions directly from current_position with
 * no kinematic translation. Used for homing axes and cartesian/core syncing.
 */
void sync_plan_position();
void sync_plan_position_e();

/**
 * Move the planner to the current position from wherever it last moved
 * (or from wherever it has been told it is located).
 */
void line_to_current_position(const feedRate_t &fr_mm_s=feedrate_mm_s);

/// Plans (non-blocking) linear move to relative distance.
/// It uses prepare_move_to_destination() for the planning which
/// is suitable with UBL.
void plan_move_by(const feedRate_t fr, const float dx, const float dy = 0, const float dz = 0, const float de = 0);

void prepare_move_to_destination(const MoveHints &hints = {});

void _internal_move_to_destination(const feedRate_t &fr_mm_s=0.0f);

inline void prepare_internal_move_to_destination(const feedRate_t &fr_mm_s=0.0f) {
  _internal_move_to_destination(fr_mm_s);
}

enum class Segmented {
    yes,
    no,
};

/// Plans (non-blocking) Z-Manhattan fast (non-linear) move to the specified location
/// Feedrate is in mm/s
/// Z-Manhattan: moves XY and Z independently. Raises before or lowers after XY motion.
/// Suitable for Z probing because it does not apply motion limits
/// Uses logical coordinates
void plan_park_move_to(const float rx, const float ry, const float rz, const feedRate_t &fr_xy, const feedRate_t &fr_z, Segmented segmented);

static inline void plan_park_move_to_xyz(const xyz_pos_t &xyz, const feedRate_t &fr_xy, const feedRate_t &fr_z, Segmented segmented) {
  plan_park_move_to(xyz.x, xyz.y, xyz.z, fr_xy, fr_z, segmented);
}

/**
 * Blocking movement and shorthand functions
 */

/**
 * Performs a blocking fast parking move to (X, Y, Z) and sets the current_position.
 * Parking (Z-Manhattan): Moves XY and Z independently. Raises Z before or lowers Z after XY motion.
 */
void do_blocking_move_to(const float rx, const float ry, const float rz, const feedRate_t &fr_mm_s=0.0f, Segmented segmented = Segmented::no);
void do_blocking_move_to(const xy_pos_t &raw, const feedRate_t &fr_mm_s=0.0f);
void do_blocking_move_to(const xyz_pos_t &raw, const feedRate_t &fr_mm_s=0.0f);
void do_blocking_move_to(const xyze_pos_t &raw, const feedRate_t &fr_mm_s=0.0f);

void do_blocking_move_to_x(const float &rx, const feedRate_t &fr_mm_s=0.0f);
void do_blocking_move_to_y(const float &ry, const feedRate_t &fr_mm_s=0.0f);
void do_blocking_move_to_z(const float &rz, const feedRate_t &fr_mm_s=0.0f, Segmented segmented = Segmented::no);

void do_blocking_move_to_xy(const float &rx, const float &ry, const feedRate_t &fr_mm_s=0.0f);
void do_blocking_move_to_xy(const xy_pos_t &raw, const feedRate_t &fr_mm_s=0.0f);
FORCE_INLINE void do_blocking_move_to_xy(const xyz_pos_t &raw, const feedRate_t &fr_mm_s=0.0f)  { do_blocking_move_to_xy(xy_pos_t(raw), fr_mm_s); }
FORCE_INLINE void do_blocking_move_to_xy(const xyze_pos_t &raw, const feedRate_t &fr_mm_s=0.0f) { do_blocking_move_to_xy(xy_pos_t(raw), fr_mm_s); }

void do_blocking_move_to_xy_z(const xy_pos_t &raw, const float &z, const feedRate_t &fr_mm_s=0.0f, Segmented segmented = Segmented::no);
FORCE_INLINE void do_blocking_move_to_xy_z(const xyz_pos_t &raw, const float &z, const feedRate_t &fr_mm_s=0.0f)  { do_blocking_move_to_xy_z(xy_pos_t(raw), z, fr_mm_s); }
FORCE_INLINE void do_blocking_move_to_xy_z(const xyze_pos_t &raw, const float &z, const feedRate_t &fr_mm_s=0.0f) { do_blocking_move_to_xy_z(xy_pos_t(raw), z, fr_mm_s); }

void remember_feedrate_and_scaling();
void remember_feedrate_scaling_off();
void restore_feedrate_and_scaling();

#if HAS_Z_AXIS
  uint8_t do_z_clearance(const float zclear, const bool lower_allowed=false);
#else
  inline uint8_t do_z_clearance(float, bool=false) { return 0; }
#endif

//
// Homing
//

uint8_t axes_need_homing(uint8_t axis_bits=0x07, AxisHomeLevel required_level = AxisHomeLevel::imprecise);
bool axis_unhomed_error(uint8_t axis_bits=0x07, AxisHomeLevel required_level = AxisHomeLevel::imprecise);

static inline bool axes_should_home(uint8_t axis_bits=0x07) { return axes_need_homing(axis_bits); }
static inline bool homing_needed_error(uint8_t axis_bits=0x07) { return axis_unhomed_error(axis_bits); }

#if ENABLED(NO_MOTION_BEFORE_HOMING)
  #define MOTION_CONDITIONS (IsRunning() && !axis_unhomed_error())
#else
  #define MOTION_CONDITIONS IsRunning()
#endif

void set_axis_is_at_home(const AxisEnum axis, bool homing_z_with_probe = true);

void set_axis_is_not_at_home(const AxisEnum axis);

void homing_failed(stdext::inplace_function<void()> fallback_error, bool crash_was_active = false, bool recover_z = false);

// Home a single logical axis
[[nodiscard]] bool homeaxis(const AxisEnum axis, const feedRate_t fr_mm_s=0.0, bool invert_home_dir = false,
  void (*enable_wavetable)(AxisEnum) = NULL, bool can_calibrate = true, bool homing_z_with_probe = true);

// Perform a single homing probe on a logical axis
float homeaxis_single_run(const AxisEnum axis, const int axis_home_dir, const feedRate_t fr_mm_s = 0.0,
  bool invert_home_dir = false, bool homing_z_with_probe = true, const int attempt = 0);

/**
 * @brief Perform a blocking, relative move on the specified axis *without* position modifiers
 * @param axis Axis to move
 * @param distance Distance relative to current position
 * @param fr_mm_s Move feedrate
 * @warning Trashes the current axis position!
 */
void do_homing_move_axis_rel(const AxisEnum axis, const float distance, const feedRate_t fr_mm_s);

// Perform a single homing move on a logical axis
uint8_t do_homing_move(const AxisEnum axis, const float distance, const feedRate_t fr_mm_s=0.0, bool can_move_back_before_homing = false, bool homing_z_with_probe = true);

struct PrepareMoveHints {
  /// Apply modifiers (MBL, skew correction, ...)
  bool apply_modifiers : 1 = true;

  /// Apply feedrate scaling
  bool scale_feedrate : 1 = true;

  MoveHints move;
};

/// Prepares the move to the target. Can apply segmentation based on MBL and other mechanisms requirements.
void prepare_move_to(const xyze_pos_t &target, feedRate_t fr_mm_s, PrepareMoveHints hints);

/**
 * Workspace offsets
 */
#if HAS_HOME_OFFSET || HAS_POSITION_SHIFT
  #if HAS_HOME_OFFSET
    extern xyz_pos_t home_offset;
  #endif
  #if HAS_POSITION_SHIFT
    extern xyz_pos_t position_shift;
  #endif
  #if HAS_HOME_OFFSET && HAS_POSITION_SHIFT
    extern xyz_pos_t workspace_offset;
    #define _WS workspace_offset
  #elif HAS_HOME_OFFSET
    #define _WS home_offset
  #else
    #define _WS position_shift
  #endif
  #if DISABLED(PRUSA_TOOLCHANGER)
    #define NATIVE_TO_LOGICAL(POS, AXIS) ((POS) + _WS[AXIS])
    #define LOGICAL_TO_NATIVE(POS, AXIS) ((POS) - _WS[AXIS])
    FORCE_INLINE void toLogical(xy_pos_t &raw)   { raw += _WS; }
    FORCE_INLINE void toLogical(xyz_pos_t &raw)  { raw += _WS; }
    FORCE_INLINE void toLogical(xyze_pos_t &raw) { raw += _WS; }
    FORCE_INLINE void toNative(xy_pos_t &raw)    { raw -= _WS; }
    FORCE_INLINE void toNative(xyz_pos_t &raw)   { raw -= _WS; }
    FORCE_INLINE void toNative(xyze_pos_t &raw)  { raw -= _WS; }
  #else
    #define NATIVE_TO_LOGICAL(POS, AXIS) ((AXIS <= Z_AXIS) ? ((POS) + _WS[AXIS] + hotend_currently_applied_offset[AXIS]) : (POS))
    #define LOGICAL_TO_NATIVE(POS, AXIS) ((AXIS <= Z_AXIS) ? ((POS) - _WS[AXIS] - hotend_currently_applied_offset[AXIS]) : (POS))
    FORCE_INLINE void toLogical(xy_pos_t &raw)   { raw += _WS + hotend_currently_applied_offset; }
    FORCE_INLINE void toLogical(xyz_pos_t &raw)  { raw += _WS + hotend_currently_applied_offset; }
    FORCE_INLINE void toLogical(xyze_pos_t &raw) { raw += _WS + hotend_currently_applied_offset; }
    FORCE_INLINE void toNative(xy_pos_t &raw)    { raw -= _WS + hotend_currently_applied_offset; }
    FORCE_INLINE void toNative(xyz_pos_t &raw)   { raw -= _WS + hotend_currently_applied_offset; }
    FORCE_INLINE void toNative(xyze_pos_t &raw)  { raw -= _WS + hotend_currently_applied_offset; }
  #endif
#else
  #define NATIVE_TO_LOGICAL(POS, AXIS) (POS)
  #define LOGICAL_TO_NATIVE(POS, AXIS) (POS)
  FORCE_INLINE void toLogical(xy_pos_t&)   {}
  FORCE_INLINE void toLogical(xyz_pos_t&)  {}
  FORCE_INLINE void toLogical(xyze_pos_t&) {}
  FORCE_INLINE void toNative(xy_pos_t&)    {}
  FORCE_INLINE void toNative(xyz_pos_t&)   {}
  FORCE_INLINE void toNative(xyze_pos_t&)  {}
#endif
#define LOGICAL_X_POSITION(POS) NATIVE_TO_LOGICAL(POS, X_AXIS)
#define LOGICAL_Y_POSITION(POS) NATIVE_TO_LOGICAL(POS, Y_AXIS)
#define LOGICAL_Z_POSITION(POS) NATIVE_TO_LOGICAL(POS, Z_AXIS)
#define RAW_X_POSITION(POS)     LOGICAL_TO_NATIVE(POS, X_AXIS)
#define RAW_Y_POSITION(POS)     LOGICAL_TO_NATIVE(POS, Y_AXIS)
#define RAW_Z_POSITION(POS)     LOGICAL_TO_NATIVE(POS, Z_AXIS)

/**
 * position_is_reachable family of functions
 */

#if 1 // CARTESIAN

  // Return true if the given position is within the machine bounds.
  inline bool position_is_reachable(const float &rx, const float &ry) {
    if (!WITHIN(ry, Y_MIN_POS - slop, Y_MAX_POS + slop)) return false;
    #if ENABLED(DUAL_X_CARRIAGE)
      if (active_extruder)
        return WITHIN(rx, X2_MIN_POS - slop, X2_MAX_POS + slop);
      else
        return WITHIN(rx, X1_MIN_POS - slop, X1_MAX_POS + slop);
    #else
      return WITHIN(rx, X_MIN_POS - slop, X_MAX_POS + slop);
    #endif
  }
  inline bool position_is_reachable(const xy_pos_t &pos) { return position_is_reachable(pos.x, pos.y); }

  #if HAS_BED_PROBE
    /**
     * Return whether the given position is within the bed, and whether the nozzle
     * can reach the position required to put the probe at the given position.
     *
     * Example: For a probe offset of -10,+10, then for the probe to reach 0,0 the
     *          nozzle must be be able to reach +10,-10.
     */
    inline bool position_is_reachable_by_probe(const float &rx, const float &ry) {
      return position_is_reachable(rx - probe_offset.x - TERN0(HAS_HOTEND_OFFSET, hotend_currently_applied_offset.x), ry - probe_offset.y - TERN0(HAS_HOTEND_OFFSET, hotend_currently_applied_offset.y))
          && WITHIN(rx, probe_min_x() - slop, probe_max_x() + slop)
          && WITHIN(ry, probe_min_y() - slop, probe_max_y() + slop);
    }
  #endif

#endif // CARTESIAN

#if !HAS_BED_PROBE
  FORCE_INLINE bool position_is_reachable_by_probe(const float &rx, const float &ry) { return position_is_reachable(rx, ry); }
#endif
FORCE_INLINE bool position_is_reachable_by_probe(const xy_int_t &pos) { return position_is_reachable_by_probe(pos.x, pos.y); }
FORCE_INLINE bool position_is_reachable_by_probe(const xy_pos_t &pos) { return position_is_reachable_by_probe(pos.x, pos.y); }

/**
 * Duplication mode
 */
#if HAS_DUPLICATION_MODE
  extern bool extruder_duplication_enabled,       // Used in Dual X mode 2
              mirrored_duplication_mode;          // Used in Dual X mode 3
  #if ENABLED(MULTI_NOZZLE_DUPLICATION)
    extern uint8_t duplication_e_mask;
  #endif
#endif

/**
 * Dual X Carriage
 */
#if ENABLED(DUAL_X_CARRIAGE)

  enum DualXMode : char {
    DXC_FULL_CONTROL_MODE,
    DXC_AUTO_PARK_MODE,
    DXC_DUPLICATION_MODE,
    DXC_MIRRORED_MODE
  };

  extern DualXMode dual_x_carriage_mode;
  extern float inactive_extruder_x_pos,           // Used in mode 0 & 1
               duplicate_extruder_x_offset;       // Used in mode 2 & 3
  extern xyz_pos_t raised_parked_position;        // Used in mode 1
  extern bool active_extruder_parked;             // Used in mode 1, 2 & 3
  extern millis_t delayed_move_time;              // Used in mode 1
  extern int16_t duplicate_extruder_temp_offset;  // Used in mode 2 & 3

  FORCE_INLINE bool dxc_is_duplicating() { return dual_x_carriage_mode >= DXC_DUPLICATION_MODE; }

  float x_home_pos(const int extruder);

  FORCE_INLINE int x_home_dir(const uint8_t extruder) { return extruder ? X2_HOME_DIR : X_HOME_DIR; }

#elif ENABLED(MULTI_NOZZLE_DUPLICATION)

  enum DualXMode : char {
    DXC_DUPLICATION_MODE = 2
  };

#else

  #define TOOL_X_HOME_DIR(T) X_HOME_DIR

#endif

#if HAS_M206_COMMAND
  void set_home_offset(const AxisEnum axis, const float v);
#endif

#if USE_SENSORLESS
  struct sensorless_t;
  sensorless_t start_sensorless_homing_per_axis(const AxisEnum axis);
  void end_sensorless_homing_per_axis(const AxisEnum axis, sensorless_t enable_stealth);
#endif
