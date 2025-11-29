#include "ceiling_clearance.hpp"

#include <marlin_server.hpp>
#include <module/planner.h>

namespace buddy {

static bool ignore_ceiling_clearance = false;

static uint8_t ceiling_clearance_active_disablers = 0;

void check_ceiling_clearance(const xyze_pos_t &target) {
    if (ignore_ceiling_clearance || ceiling_clearance_active_disablers) {
        return;
    }

    if (planner.max_printed_z - target.z <= Z_CEILING_CLEARANCE) {
        // The motion is safe -> do nothing
        return;
    }

    // User clicks on continue -> let the printer do the move and do not show any further
    if (marlin_server::prompt_warning(WarningType::CeilingClearanceViolation) == Response::Continue) {
        ignore_ceiling_clearance = true;
        return;
    }

    // Wait for the all queued moves to finish
    planner.synchronize();

    // We are currently not moving, but this will prevent from the current command from issuing any moves
    // The planning shouldl automatically resume in the outer marlin loop
    planner.quick_stop();

    // Abort the current print
    marlin_server::print_abort();
}

void reenable_ceiling_clearance_warning() {
    ignore_ceiling_clearance = false;
}

CeilingClearanceCheckDisabler::CeilingClearanceCheckDisabler() {
    ceiling_clearance_active_disablers++;
}

CeilingClearanceCheckDisabler::~CeilingClearanceCheckDisabler() {
    ceiling_clearance_active_disablers--;
}

} // namespace buddy
