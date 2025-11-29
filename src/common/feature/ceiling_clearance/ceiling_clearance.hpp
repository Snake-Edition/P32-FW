#pragma once

#include <core/types.h>

namespace buddy {

/// Checks if the movement to the \param target position wouldn't violate ceiling clearance rules
/// If yes, possibly blocks the movement and aborts print
/// To be called from the marlin thread only.
void check_ceiling_clearance(const xyze_pos_t &target);

/// Re-enable ceiling clearance warning after the user potentially ignored the previous one
/// To be called from the marlin thread only.
void reenable_ceiling_clearance_warning();

class CeilingClearanceCheckDisabler {
public:
    CeilingClearanceCheckDisabler();
    CeilingClearanceCheckDisabler(CeilingClearanceCheckDisabler &&) = delete;
    ~CeilingClearanceCheckDisabler();
};

} // namespace buddy
