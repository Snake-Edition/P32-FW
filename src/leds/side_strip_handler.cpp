#include "leds/side_strip_handler.hpp"
#include "leds/simple_transition_controller.hpp"

namespace leds {

static SimpleTransitionController &controller_instance() {
    static SimpleTransitionController instance;
    return instance;
}

SideStripHandler &SideStripHandler::instance() {
    static SideStripHandler instance;
    return instance;
}

SideStripHandler::SideStripHandler() {
    load_config();
}

void SideStripHandler::activity_ping() {
    std::lock_guard lock(mutex);
    active_timestamp_ms = ticks_ms();
}

void SideStripHandler::set_custom_color(ColorRGBW color, uint32_t duration_ms, uint32_t transition_ms) {
    std::lock_guard lock(mutex);
    custom_color.emplace(color, ticks_ms(), duration_ms, transition_ms);
    // Set state temporarily to off to force a change in case the state is already custom_color
    state = SideStripState::off;
}

void SideStripHandler::update() {
    std::lock_guard lock(mutex);
    const uint32_t time_ms = ticks_ms();

    auto &controller = controller_instance();
    if (custom_color && time_ms - custom_color->start_ms < custom_color->duration_ms) {
        change_state(SideStripState::custom_color);
    } else {
        custom_color.reset();

        if (!dimming_enabled || (active_timestamp_ms != 0 && time_ms - active_timestamp_ms < active_timeout_ms)) {
            change_state(SideStripState::active);
        } else {
            // Clear to prevent overflow bugs
            active_timestamp_ms = 0;
            change_state(SideStripState::idle);
        }
    }

    controller.update();
}

void SideStripHandler::load_config() {
    std::lock_guard lock(mutex);
#if HAS_XBUDDY_EXTENSION()
    camera_enabled = config_store().xbe_usb_power.get();
    brightness = config_store().side_leds_max_brightness.get();
    if (camera_enabled) {
        dimming_enabled = config_store().side_leds_dimming_enabled_with_camera.get();
    } else
#endif
    {
        dimming_enabled = config_store().side_leds_dimming_enabled.get();
    }
    // Set state to off to force a change of state that will transition to the new brightness
    state = SideStripState::off;
}

uint8_t SideStripHandler::get_max_brightness() const {
    std::lock_guard lock(mutex);
    return brightness;
}

void SideStripHandler::set_max_brightness(uint8_t value) {
    config_store().side_leds_max_brightness.set(value);
    std::lock_guard lock(mutex);
    if (brightness == value) {
        return;
    }

    brightness = value;
    // Set state to off to force a change of state that will transition to the new brightness
    state = SideStripState::off;
}

bool SideStripHandler::get_dimming_enabled() const {
    std::lock_guard lock(mutex);
    return dimming_enabled;
}

void SideStripHandler::set_dimming_enabled(bool value) {
#if HAS_XBUDDY_EXTENSION()
    if (camera_enabled) {
        config_store().side_leds_dimming_enabled_with_camera.set(value);
    } else
#endif
    {
        config_store().side_leds_dimming_enabled.set(value);
    }
    std::lock_guard lock(mutex);
    dimming_enabled = value;
}

leds::ColorRGBW SideStripHandler::color() const {
    std::lock_guard lock(mutex);
    return controller_instance().color();
}

void SideStripHandler::change_state(SideStripState state) {
    if (this->state != state) {
        this->state = state;

        uint32_t duration;
        if (state == SideStripState::idle) {
            duration = 5000;
        } else if (state == SideStripState::custom_color) {
            duration = custom_color->transition_ms;
        } else {
            duration = 500;
        }
        controller_instance().set(get_color_for_state(state), duration);
    }
}

ColorRGBW SideStripHandler::get_color_for_state(SideStripState state) {
    switch (state) {
    case SideStripState::off:
        return ColorRGBW();
    case SideStripState::idle:
        if constexpr (has_white_led()) {
            return ColorRGBW(0, 0, 0, 40).clamp(brightness);
        } else {
            return ColorRGBW(40, 40, 40).clamp(brightness);
        }
    case SideStripState::active:
        if constexpr (has_white_led()) {
            return ColorRGBW(0, 0, 0, 255).clamp(brightness);
        } else {
            return ColorRGBW(255, 255, 255).clamp(brightness);
        }
    case SideStripState::custom_color:
        return ColorRGBW(custom_color->color);
    }

    return {};
}

} // namespace leds
