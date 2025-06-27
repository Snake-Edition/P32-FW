#include "display_hw_checks.hpp"

#include <display.hpp>
#include <ScreenHandler.hpp>
#include <option/has_touch.h>
#include <device/peripherals.h>
#include <logging/log.hpp>
#include <utils/timing/rate_limiter.hpp>

LOG_COMPONENT_REF(GUI);

#if HAS_TOUCH()
    #include <hw/touchscreen/touchscreen.hpp>
#endif

void lcd::communication_check() {
    static RateLimiter<uint32_t> check_limiter { 2048 };
    if (!check_limiter.check(ticks_ms())) {
        return;
    }

    bool do_reset = display::is_reset_required();

#if HAS_TOUCH()
    if (touchscreen.is_enabled()) {
        touchscreen.perform_check();
        do_reset |= (touchscreen.required_recovery_action() == Touchscreen_GT911::RecoveryAction::restart_display);
    }
#endif

    if (!do_reset) {
        return;
    }

    display::complete_reinit();
    display::init();
    if (auto *screen = Screens::Access()->Get()) {
        screen->Invalidate();
    }
}
