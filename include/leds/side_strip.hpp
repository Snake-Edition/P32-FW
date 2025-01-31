#pragma once
#include "led_strip.hpp"
#include <assert.h>
#include <hw/neopixel.hpp>
#include "printers.h"
#include <option/xl_enclosure_support.h>

#ifndef UNITTESTS
    #include "led_lcd_cs_selector.hpp"
#endif

namespace leds {

class SideStrip : public LedStrip {
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

    int GetLedCount() {
        return 1;
    }

    ColorRGBW GetColor([[maybe_unused]] int led_idx) {
        assert(led_idx == 0);
        return current_color;
    }

    void SetColor([[maybe_unused]] int led_idx, ColorRGBW color) {
        assert(led_idx == 0);
        SetColor(color);
    }

    void SetColor(ColorRGBW color) {
        if (color == current_color) {
            return;
        }

        current_color = color;
        needs_update = true;
    }

#if XL_ENCLOSURE_SUPPORT()
    void SetEnclosureFanPwm(uint8_t pwm) {
        if (enclosure_fan_pwm != pwm) {
            enclosure_fan_pwm = pwm;
            needs_update = true;
        }
    }
#endif // XL_ENCLOSURE_SUPPORT

    void Update();

private:
    ColorRGBW current_color;
    std::atomic<uint8_t> enclosure_fan_pwm = 0;
    std::atomic<bool> needs_update = false;

#if PRINTER_IS_PRUSA_XL()
    /// First driver in the daisy chain: RGB, second driver: W + enclosure fan
    static constexpr size_t led_drivers_count = 2;
    neopixel::LedsSPI10M5Hz<led_drivers_count, SideStripWriter::write> leds;

#elif PRINTER_IS_PRUSA_iX()
    /// 3x 3 or 3 x 6 RGB drivers in the U shape along the gantry (left, back, right)
    /// Newer strips have double the segments (the 3 x 6 version), just
    /// unconditionally send data for the variant with more segments
    static constexpr size_t led_drivers_count = 18;
    neopixel::LedsSPI10M5Hz<led_drivers_count, SideStripWriter::write> leds;

#elif PRINTER_IS_PRUSA_COREONE() || defined(UNITTESTS)
    // Single white-only driver

#else
    #error "Not defined for this printer."
#endif
};

extern SideStrip side_strip;

} // namespace leds
