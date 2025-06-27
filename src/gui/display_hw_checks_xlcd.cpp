#include "display_hw_checks.hpp"

#include <display.hpp>
#include <ScreenHandler.hpp>
#include <option/has_touch.h>
#include <device/peripherals.h>
#include <logging/log.hpp>
#include <utils/timing/rate_limiter.hpp>
#include <utils/timing/combo_counter.hpp>
#include <marlin_client.hpp>

LOG_COMPONENT_REF(GUI);

#if HAS_TOUCH()
    #include <hw/touchscreen/touchscreen.hpp>
#endif

void lcd::communication_check() {
    static RateLimiter<uint32_t> check_limiter { 2048 };
    const auto now = ticks_ms();
    if (!check_limiter.check(now)) {
        return;
    }

    bool do_reset = display::is_reset_required();

    // Track consecutive display errors
    static ComboCounter<uint32_t, uint8_t> error_combo_counter { 10'000 };
    error_combo_counter.step(now, do_reset);

#if HAS_TOUCH()
    if (touchscreen.is_enabled()) {
        touchscreen.perform_check();
        do_reset |= (touchscreen.required_recovery_action() == Touchscreen_GT911::RecoveryAction::restart_display);
    }
#endif

    if (!do_reset) {
        return;
    }

    // If there's too many consecutive display errors, switch to lower refresh rate
    if (error_combo_counter.current_combo() >= 3 && !config_store().reduce_display_baudrate.get()) {
        // WarningType::DisplayProblemDetected is asking whether we should lower the refresh rate or not,
        // but we should reduce the baudrate right now (and then potentially revert if back if necessary).
        // The ongoing display issues might potentially make the warning unreadable otherwise
        // The full refresh rate should be set again if the user selects "No" in the warning.
        config_store().reduce_display_baudrate.set(true);
        marlin_client::set_warning(WarningType::DisplayProblemDetected);
        error_combo_counter.reset();
    }

    display::complete_reinit();
    display::init();
    if (auto *screen = Screens::Access()->Get()) {
        screen->Invalidate();
    }
}
