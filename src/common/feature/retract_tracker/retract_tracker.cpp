#include "retract_tracker.hpp"
#include <marlin_vars.hpp>

using namespace buddy;

RetractTracker::RetractTracker() {
    reset();
}

void RetractTracker::track_extruder_move(float extrusion_distance) {
    const auto hotend = marlin_vars().active_hotend_id();
    const float new_retraction_distance = std::clamp(retracted_distances[hotend] - /* inverting to positive */ extrusion_distance, 0.0f, extruder_to_nozzle_distance);
    retracted_distances[hotend] = new_retraction_distance;

    if (!distance_valid.test(hotend) && new_retraction_distance == 0.0f) {
        // retracted_distance resets to extruder_to_nozzle_distance and we validate when it reaches zero - we are sure filament is fully in the nozzle
        distance_valid.set(hotend, true);
    } else if (distance_valid.test(hotend) && new_retraction_distance == extruder_to_nozzle_distance) {
        // if we retract more than extruder_to_nozzle_distance, we most likely lost the track of the retractions, because the filament is no longer engaged with the extruder
        distance_valid.set(hotend, false);
    }
}

void RetractTracker::reset() {
    for (uint8_t i = 0; i < HOTENDS; i++) {
        // Starts as if it is fully retracted (to the edge of the extruder)
        // After the value validates (travels at least +extruder_to_nozzle_distance) we can start to keep track of retracted distance
        retracted_distances[i] = extruder_to_nozzle_distance;
    }
    distance_valid.reset();
}

std::optional<float> RetractTracker::get_retracted_distance(uint8_t hotend) const {
    assert(hotend < HOTENDS);
    if (!distance_valid.test(hotend)) {
        return std::nullopt;
    }
    return retracted_distances[hotend];
}

RetractTracker &buddy::retract_tracker() {
    static RetractTracker instance;
    return instance;
}
