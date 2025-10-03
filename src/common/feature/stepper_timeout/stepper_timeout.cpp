#include "stepper_timeout.hpp"

#include <option/board_is_master_board.h>
#include <Configuration.h>
#include <marlin_server.hpp>
#include <module/stepper/indirection.h>

namespace buddy {

StepperTimeoutManager &stepper_timeout() {
    static StepperTimeoutManager instance;
    return instance;
}

StepperTimeoutManager::StepperTimeoutManager()
    : timer_((DEFAULT_STEPPER_DEACTIVE_TIME)*1000U) {
}

void StepperTimeoutManager::reset() {
    timer_.restart(ticks_ms());
}

void StepperTimeoutManager::step() {
    const auto now = ticks_ms();

    // Any activity - running gcode, motors, ...
    if (marlin_server::is_processing()) {
        timer_.restart(now);
    }

    if (timer_.check(now)) {
        trigger();
    }
}

void StepperTimeoutManager::trigger() {
    // In case the trigger was called explicitly from somewhere
    timer_.stop();

#if _DEBUG
    // Report steppers being disabled to the user
    // Skip if position not trusted to avoid warnings when position is not important
    if (axes_home_level != AxesHomeLevel::no_axes_homed) {
        marlin_server::set_warning(WarningType::SteppersTimeout);
    }
#endif

#if (ENABLED(XY_LINKED_ENABLE) && (ENABLED(DISABLE_INACTIVE_X) || ENABLED(DISABLE_INACTIVE_Y)))
    disable_XY();
#else
    #if ENABLED(DISABLE_INACTIVE_X)
    disable_X();
    #endif
    #if ENABLED(DISABLE_INACTIVE_Y)
    disable_Y();
    #endif
#endif
#if ENABLED(DISABLE_INACTIVE_Z)
    disable_Z();
#endif
#if ENABLED(DISABLE_INACTIVE_E)
    disable_e_steppers();
#endif
}

} // namespace buddy
