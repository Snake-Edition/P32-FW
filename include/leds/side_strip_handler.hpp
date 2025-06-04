#pragma once

#include "color.hpp"
#include "printers.h"

#include <freertos/mutex.hpp>
#include <optional>

namespace leds {

enum class SideStripState {
    off,
    idle,
    active,
    custom_color,
    _last = custom_color,
};

class SideStripHandler {
public:
    static constexpr bool has_white_led_and_enclosure_on_second_driver() {
#if PRINTER_IS_PRUSA_XL()
        return true;
#else
        return false;
#endif
    }

    static constexpr bool has_white_led() {
#if PRINTER_IS_PRUSA_COREONE()
        return true;
#else
        return has_white_led_and_enclosure_on_second_driver();
#endif
    }

    static SideStripHandler &instance();

    SideStripHandler();

    void activity_ping();

    void set_custom_color(ColorRGBW color, uint32_t duration_ms, uint32_t transition_ms);

    void update();

    /// Set maximum brightness of the side leds white channel. The W brightness is limited to this value.
    /// 0 = disable side leds completely (even RGB status blinking), 255 = full
    uint8_t get_max_brightness() const;
    void set_max_brightness(uint8_t value);

    bool get_dimming_enabled() const;
    void set_dimming_enabled(bool value);

    leds::ColorRGBW color() const;

private:
    static constexpr uint32_t active_timeout_ms = 120 * 1000;

    void change_state(SideStripState state);

    ColorRGBW get_color_for_state(SideStripState state);

    struct CustomColorState {
        ColorRGBW color;
        uint32_t start_ms;
        uint32_t duration_ms;
        uint32_t transition_ms;
    };

    mutable freertos::Mutex mutex;
    bool dimming_enabled { true };
    uint8_t brightness { 255 };

    SideStripState state = SideStripState::off;
    uint32_t active_timestamp_ms = 0; // Timestamp of the last activity for idle dimming
    std::optional<CustomColorState> custom_color;
};

} // namespace leds
