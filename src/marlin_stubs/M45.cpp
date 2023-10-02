#include "PrusaGcodeSuite.hpp"

#include "../../lib/Marlin/Marlin/src/inc/MarlinConfig.h"
#include "../../lib/Marlin/Marlin/src/gcode/gcode.h"
#include "../../lib/Marlin/Marlin/src/feature/bedlevel/bedlevel.h"
#include "../../lib/Marlin/Marlin/src/module/motion.h"
#include "../../lib/Marlin/Marlin/src/module/planner.h"
#include "../../lib/Marlin/Marlin/src/module/stepper.h"
#include "../../lib/Marlin/Marlin/src/module/probe.h"
#include "../../lib/Marlin/Marlin/src/gcode/queue.h"
#include "../../lib/Marlin/Marlin/src/gcode/motion/G2_G3.h"
#include "../../lib/Marlin/Marlin/src/module/endstops.h"

const float radius_8 = 3.f;

xy_pos_t get_skew_point(int8_t ix, int8_t iy) {
    // std::array<xy_uint8_t, 9> skew_points = { { 14, 20 }, { 74, 20 }, { 146, 20 }, { 146, 89 }, { 74, 89 }, { 14, 99 }, { 14, 164 }, { 74, 164 }, { 146, 164 } };
    const xyz_pos_t probe = NOZZLE_TO_PROBE_OFFSET;

    if (ix == 0 && iy == 1)
        return xy_pos_t { 14 - probe.x, 99 - probe.y };

    xy_pos_t pos;
    switch (ix) {
    case 0:
        pos.x = 14 - probe.x;
        break;
    case 1:
        pos.x = 74 - probe.x;
        break;
    case 2:
        pos.x = 146 - probe.x;
        break;
    }

    switch (iy) {
    case 0:
        pos.y = 20 - probe.y;
        break;
    case 1:
        pos.y = 89 - probe.y;
        break;
    case 2:
        pos.y = 164 - probe.y;
        break;
    }
    return pos;
}

xy_pos_t get_skew_point(xy_byte_t index) { return get_skew_point(index.x, index.y); }

void print_area(std::array<std::array<float, 32>, 32> z_grid) {
    SERIAL_EOL();
    for (int8_t y = 0; y < 32; ++y) {
        for (int8_t x = 0; x < 32; ++x) {
            SERIAL_ECHO(z_grid[x][y]);
            SERIAL_CHAR(' ');
        }
        SERIAL_EOL();
    }
    SERIAL_EOL();
}

xy_pos_t calculate_center(std::array<std::array<float, 32>, 32> z_grid) {
    /// TODO:
    return {};
}

void print_centers(std::array<std::array<xy_pos_t, 3>, 3> centers) {
    SERIAL_EOL();
    for (int8_t y = 0; y < 3; ++y) {
        for (int8_t x = 0; x < 3; ++x) {
            SERIAL_CHAR('[');
            SERIAL_ECHO(centers[x][y].x);
            SERIAL_CHAR(',');
            SERIAL_ECHO(centers[x][y].y);
            SERIAL_CHAR(']');
            SERIAL_CHAR(' ');
        }
        SERIAL_EOL();
    }
    SERIAL_EOL();
}

void apply_transform(xy_pos_t &point, float co, float si, float skew) {
    xy_pos_t tmp;
    tmp.y = si * point.x + co * point.y;
    tmp.x = co * point.x - si * point.y + skew * tmp.y;
    point = tmp;
}

float get_length(xy_pos_t p1, xy_pos_t p2, float co, float si, float skew) {
    apply_transform(p1, co, si, skew);
    apply_transform(p2, co, si, skew);
    return sqrt(sq(p1.x - p2.x) + sq(p1.y - p2.y));
}

float get_error(std::array<xy_byte_t, 4> indexes, std::array<xy_pos_t, 4> measured, float co, float si, float skew) {
    std::array<float, 2> lm, lc;
    for (int i = 0; i < 2; ++i) {
        lm[i] = get_length(measured[2 * i], measured[2 * i + 1], co, si, skew);
        lc[i] = get_length(get_skew_point(indexes[2 * i]), get_skew_point(indexes[2 * i + 1]), co, si, skew);
    }

    return sq(lm[0] - lc[0]) + sq(lm[1] - lc[1]);
}

float find_skew(std::array<xy_byte_t, 4> indexes, std::array<xy_pos_t, 4> points, float rotation, float &skew) {
    const float co = cos(rotation);
    const float si = sin(rotation);

    // try different skews and find best result
    float best_err = std::numeric_limits<float>::max();
    float best_skew = 0.f;
    float step = 0.01f;
    const float eps = 0.0001f;
    float err1 = 0.f;
    float err3 = 0.f;

    // find three values:
    // if middle is the best then we are at the minimum (increase precision / decrease step size)
    // if a side is the best then go that direction until the direction changes

    // optimization: the middle stays the same if step is changed (no need for recompute)
    float err2 = get_error(indexes, points, co, si, skew);
    while (step > eps) {
        err1 = get_error(indexes, points, co, si, skew - step);
        err3 = get_error(indexes, points, co, si, skew + step);

        int dir = (err3 < err1) ? 1 : -1;

        while (1) {
            if (err2 < err1 && err2 < err3) {
                if (err2 < best_err) {
                    best_err = err2;
                    best_skew = skew;
                }
                break;
            }

            /// TODO: convert err to array and use indexes
            if (err1 < err3) {
                if (err1 < best_err) {
                    best_err = err1;
                    best_skew = skew - step;
                }

            } else {
                if (err3 < best_err) {
                    best_err = err3;
                    best_skew = skew + step;
                }
            }

            int dir_new = (err3 < err1) ? 1 : -1;
            if (dir_new != dir)
                break;

            // continue in the same direction
            skew += dir * step;
            if (dir == 1) {
                err1 = err2;
                err2 = err3;
                err3 = get_error(indexes, points, co, si, skew + step);
            } else {
                err3 = err2;
                err2 = err1;
                err1 = get_error(indexes, points, co, si, skew - step);
            }
        }
        // we are close to minimum => decrease step
        step *= 0.5f;
    }

    skew = best_skew;
    return best_err;
}

void find_rotation_skew(std::array<xy_byte_t, 4> indexes, std::array<xy_pos_t, 4> measured, float &rotation, float &skew) {
    // try different rotation angles and find proper skew for the specific rotation
    float best_err = std::numeric_limits<float>::max();
    float best_skew = 0.f;
    float best_rotation = 0.f;
    float rot_step = 0.25f;
    const float rot_eps = 0.00025f;
    float err1 = 0.f;
    float err3 = 0.f;
    float skew1 = 0.f;
    float skew2 = 0.f;
    float skew3 = 0.f;

    // find three values:
    // if middle is the best then we are at the minimum (increase precision / decrease step size)
    // if a side is the best then go that direction until the direction changes

    // optimization: the middle stays the same if step is changed (no need for recompute)
    float err2 = find_skew(indexes, measured, rotation, skew2);
    while (rot_step > rot_eps) {
        err1 = find_skew(indexes, measured, rotation - rot_step, skew1);
        err3 = find_skew(indexes, measured, rotation + rot_step, skew3);

        int dir = (err3 < err1) ? 1 : -1;

        while (1) {
            if (err2 < err1 && err2 < err3) {
                if (err2 < best_err) {
                    best_err = err2;
                    best_rotation = rotation;
                    best_skew = skew2;
                }
                break;
            }

            /// TODO: convert err and skew to arrays and use indexes
            if (err1 < err3) {
                if (err1 < best_err) {
                    best_err = err1;
                    best_rotation = rotation - rot_step;
                    best_skew = skew1;
                }

            } else {
                if (err3 < best_err) {
                    best_err = err3;
                    best_rotation = rotation + rot_step;
                    best_skew = skew3;
                }
            }

            int dir_new = (err3 < err1) ? 1 : -1;
            if (dir_new != dir)
                break;

            // continue in the same direction
            rotation += dir * rot_step;
            if (dir == 1) {
                err1 = err2;
                skew1 = skew2;
                err2 = err3;
                skew2 = skew3;
                err3 = find_skew(indexes, measured, rotation + rot_step, skew3);
            } else {
                err3 = err2;
                skew3 = skew2;
                err2 = err1;
                skew2 = skew1;
                err1 = find_skew(indexes, measured, rotation - rot_step, skew);
            }
        }
        // we are close to minimum => decrease step
        rot_step *= 0.5f;
    }

    rotation = best_rotation;
    skew = best_skew;
}

void calculate_skews(std::array<std::array<xy_pos_t, 3>, 3> centers) {
    // generate all usable pairs of pairs of points (2 line segments)
    // usable = with some angle between line segments (for stable computation)
    // don't use the same (flipped) vectors' pair again
    int pairs = 0;
    float rotation, skew;
    for (uint8_t y0 = 0; y0 < 3; ++y0) {
        for (uint8_t x0 = 0; x0 < 3; ++x0) {
            for (uint8_t y1 = y0; y1 < 3; ++y1) {
                for (uint8_t x1 = x0 + 1; x1 < 3; ++x1) {
                    for (uint8_t y2 = y0; y2 < 3; ++y2) {
                        for (uint8_t x2 = x0; x2 < 3; ++x2) {
                            for (uint8_t y3 = y1; y3 < 3; ++y3) {
                                for (uint8_t x3 = x2 + 1; x3 < 3; ++x3) {
                                    float l1 = sqrt(sq(x1 - x0) + sq(y1 - y0));
                                    float l2 = sqrt(sq(x3 - x2) + sq(y3 - y2));
                                    float cos_angle = ((x1 - x0) * (x3 - x2) + (y1 - y0) * (y3 - y2)) / (l1 * l2);
                                    if (cos_angle >= 0.99f) //< nearly parallel vectors => unstable result
                                        continue;
                                    ++pairs;
                                    std::array<xy_byte_t, 4> indexes = { xy_byte_t { x0, y0 }, xy_byte_t { x1, y1 }, xy_byte_t { x2, y2 }, xy_byte_t { x3, y3 } };
                                    std::array<xy_pos_t, 4> measured = { centers[x0][y0], centers[x1][y1], centers[x2][y2], centers[x3][y3] };
                                    find_rotation_skew(indexes, measured, rotation, skew);
                                    /// TODO:
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

static float run_z_probe(float min_z = -1) {
    // Probe downward slowly to find the bed
    if (do_probe_move(min_z, MMM_TO_MMS(Z_PROBE_SPEED_SLOW)))
        return NAN;
    return current_position.z;
}

float probe_at_skew_point(const xy_pos_t &pos) {
    xyz_pos_t npos = { pos.x, pos.y };
    npos.z = current_position.z;
    if (!position_is_reachable_by_probe(npos))
        return NAN; // The given position is in terms of the probe

    const float old_feedrate_mm_s = feedrate_mm_s;
    feedrate_mm_s = XY_PROBE_FEEDRATE_MM_S;

    // Move the probe to the starting XYZ
    do_blocking_move_to(npos);

    float measured_z = run_z_probe();

    /// TODO: raise until untriggered
    do_blocking_move_to_z(npos.z + (Z_CLEARANCE_BETWEEN_PROBES), MMM_TO_MMS(Z_PROBE_SPEED_FAST));

    feedrate_mm_s = old_feedrate_mm_s;
    if (isnan(measured_z))
        return 0;
    return measured_z;
}

// Probe around reference point to get save Z
// \returns true if probe triggered
bool find_safe_z() {
    bool hit = false;
    xyz_pos_t ref_pos = current_position;
    float z = current_position.z;
    const float old_feedrate_mm_s = feedrate_mm_s;
    feedrate_mm_s = XY_PROBE_FEEDRATE_MM_S;
    float measured_z = 2.f;

    for (; z >= 0; z -= .33f) {
        for (int y = -1; y <= 1; ++y) {
            for (int x = -1; x <= 1; ++x) {
                // Move the probe to the XY position
                xyz_pos_t pos = { 3.f * x, 3.f * y, 0.f };
                pos += ref_pos;
                do_blocking_move_to(pos);

                // probe down
                endstops.enable(true);
                measured_z = run_z_probe(z);
                endstops.not_homing();

                // Check to see if the probe was triggered
                hit = TEST(endstops.trigger_state(), Z_MIN);
                if (hit)
                    break;

                // move up
                do_blocking_move_to_z(current_position.z + 1.5f, MMM_TO_MMS(Z_PROBE_SPEED_FAST));
            }
        }
    }

    do_blocking_move_to_z(measured_z, MMM_TO_MMS(Z_PROBE_SPEED_FAST));
    feedrate_mm_s = old_feedrate_mm_s;
    // Clear endstop flags
    endstops.hit_on_purpose();
    // Get Z where the steppers were interrupted
    set_current_from_steppers_for_axis(Z_AXIS);
    // Tell the planner where we actually are
    sync_plan_position();
    return hit;
}

void PrusaGcodeSuite::M45() {
    // TODO G28 if needed
    if (axis_unhomed_error())
        return;

    planner.synchronize();
    set_bed_leveling_enabled(false);
    remember_feedrate_scaling_off();

    xy_pos_t probePos;
    xy_probe_feedrate_mm_s = MMM_TO_MMS(XY_PROBE_SPEED);
    // std::array<std::array<float, 32>, 32> z_grid;
    std::array<std::array<xy_pos_t, 3>, 3> centers;

    /// cycle over 9 points
    for (int8_t py = 0; py < 3; ++py) {
        for (int8_t px = 0; px < 3; ++px) {
            probePos = get_skew_point(px, py);
            SERIAL_ECHO(current_position.z);
            SERIAL_EOL();
            do_blocking_move_to_z(2, MMM_TO_MMS(Z_PROBE_SPEED_FAST));
            SERIAL_ECHO(current_position.z);
            SERIAL_EOL();
            do_blocking_move_to(probePos, xy_probe_feedrate_mm_s);
            SERIAL_ECHO(current_position.z);
            SERIAL_EOL();
            bool is_z = find_safe_z();
            SERIAL_ECHO(current_position.z);
            SERIAL_EOL();

            if (!is_z) {
                centers[px][py] = xy_pos_t { NAN, NAN };
                continue;
            }

            // /// scan 32x32 array
            // for (int8_t y = 0; y < 32; ++y) {
            //     for (int8_t x = 0; x < 32; ++x) {
            //         probePos.x += x - 32 / 2 + .5f;
            //         probePos.y += y - 32 / 2 + .5f;
            //         /// FIXME: don't go too low
            //         float measured_z = probe_at_skew_point(probePos);
            //         z_grid[x][y] = isnan(measured_z) ? -100.f : measured_z;
            //         idle(false);
            //     }
            // }

            // /// print point grid
            // // print_2d_array(z_grid.size(), z_grid[0].size(), 3, [](const uint8_t ix, const uint8_t iy) { return z_grid[ix][iy]; });

            // print_area(z_grid);

            // //                 SERIAL_ECHO(int(x));
            // //   SERIAL_EOL();
            // //   SERIAL_ECHOLNPGM("measured_z = ["); // open 2D array

            // centers[px][py] = calculate_center(z_grid);
        }
    }

    // print_centers(centers);

    current_position.z -= bilinear_z_offset(current_position);
    planner.leveling_active = true;
    restore_feedrate_and_scaling();
    if (planner.leveling_active)
        sync_plan_position();
    idle(false);
    /// TODO: convert to non-blocking move
    move_z_after_probing();

    // calculate_skews(centers);
    /// pick median
    /// set XY skew

    planner.synchronize();
    report_current_position();
}

/// Skew computation:

/// M  ... measured points (points in columns) (XY)
/// I  ... ideal point position matrix (points in columns) (XY)
/// SH1... 1st shift matrix (2 variables)
/// R  ... rotation matrix (1 variable)
/// SH2... 2nd shift matrix (2 variables)
/// SK ... skew matrix (1 variable)

/// SK =  1 sk
///       0  1

/// SK * (SH2 + R * (SH1 + I)) = M

/// sh2x + co * (sh1x + ix) - si * (sh1y + iy) + sk * my  = mx
/// sh2y + si * (sh1x + ix) + co * (sh1y + iy)            = my
/// co = cos(phi), si = sin(phi)

/// 3 (XY) points needed to fully define all 6 variables.
/// Too complicated. But distance (of 2 points) is not dependent on shift or rotation.
/// Still, we need rotation to apply proper skew.

/// B  ... base vectors of points (shifted to [0,0]) (XY)

/// ||SK * R * B|| = ||M||

/// px = co * bx - si * by   // x part
/// py = si * bx + co * by   // y part
/// ||px + sk * py, py|| = ||mx, my||

/// Ax + b = 0
///  ||px1 + sk * py1, py1|| - ||mx1, my1|| = 0
///  ||px2 + sk * py2, py2|| - ||mx2, my2|| = 0

/// Now, 2 vectors (3 or 4 points) provide over-defined equation which eliminates some error.
/// Ideally, vectors should not be in similar direction for precise results (right angle is the best).
/// Error plane should have at most 2 local minimas which is ideal for iterative numerical method.
/// We can set rotation limits to +/- 20° because it will be, typically, only few degrees.
/// Use golden ratio 0.618.
/// Guess rotation and compute skew coefficient. Minimize error.

/// err = (px + sk * py)^2 + py^2 - (mx^2 + my^2)
/// err = px^2 + sk^2 * py^2 + 2 * px * sk * py + py^2 - mx^2 - my^2

/// err = sk^2 * (py1^2) + sk * (2 * px1 * py1) + (py1^2 + px1^2 - mx1^2 - my1^2)
/// err = sk^2 * (py2^2) + sk * (2 * px2 * py2) + (py2^2 + px2^2 - mx2^2 - my2^2)
