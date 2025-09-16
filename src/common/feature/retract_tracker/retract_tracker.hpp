/// \file

#pragma once

#include <array>
#include <utils/uncopyable.hpp>
#include <bitset>
#include <optional>
#include <option/has_loadcell.h>

namespace buddy {

/**
 *  Passive observer that keeps track of retracted distance on each hotend during printing
 *
 *  On XL, retracting and ramming is handled by PrusaSlicer, which invalidates filament_retracted_distances in persistent storage
 *  and therefore collides with auto_retract feature. This tracker keeps track of the retracted distance on each hotend during printing
 *  and on print finish/abort/stop it saves the values of respected hotends in the persistent storage
 *
 *  Retracted distances are saved in range < 0 ; 254 > (mm)
 *  Value 255 is reserved as invalid / unknown value
 */
class RetractTracker : Uncopyable {
    friend RetractTracker &retract_tracker();

#if HAS_LOADCELL() // Instead of HAS_NEXTRUDER() which is not present in RELEASE-6.4
    static constexpr float extruder_to_nozzle_distance = 40.f; // mm
#else
    #error
#endif

public:
    /// Adds \param extrusion_distance (retraction is negative) to temporary value of active hotend
    void track_extruder_move(float extrusion_distance);

    /// Return temporary value (retraction is positive) of \param hotend
    std::optional<float> get_retracted_distance(uint8_t hotend) const;

    /// Forget all hotends (invalidate and set back to extruder_to_nozzle_distance)
    void reset();

private:
    RetractTracker();
    std::array<float, HOTENDS> retracted_distances;
    std::bitset<HOTENDS> distance_valid; ///< The hotend validates  by traveling at least +extruder_to_nozzle_distance
};

RetractTracker &retract_tracker();

} // namespace buddy
