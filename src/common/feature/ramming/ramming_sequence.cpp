#include "ramming_sequence.hpp"

#include <module/planner.h>
#include <mapi/motion.hpp>
#include <core/types.h>

using namespace buddy;

bool RammingSequence::execute(const InterruptCallback &callback) const {
    // Use default motion parameters for the ramming sequence
    Temporary_Reset_Motion_Parameters mp_guard;

    for (const auto &step : steps()) {
        mapi::extruder_move(step.e, MMM_TO_MMS(step.fr_mm_min));

        if (!callback()) {
            return false;
        }
    }

    planner.synchronize();
    return !planner.draining();
}

bool RammingSequence::undo(float feedrate_mm_s) const {
    mapi::extruder_move(retracted_distance(), feedrate_mm_s);
    planner.synchronize();
    return !planner.draining();
}
