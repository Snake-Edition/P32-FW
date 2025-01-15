#include <leds/side_strip.hpp>
#include <device/hal.h>
#include <device/peripherals.h>
#include <cmsis_os.h>
#include "neopixel.hpp"

using namespace leds;

SideStrip leds::side_strip;

void SideStrip::Update() {
    if (!needs_update) {
        return;
    }
    needs_update = false;

#if PRINTER_IS_PRUSA_COREONE()
    // Polled in XBuddyExtension::step()

#else
    if (has_white_led_and_enclosure_on_second_driver()) {
        // On XL, there are two neopixel drivers, the first one controls RGB of
        // the RGBW LED strip. The second one controls the white color of the
        // RGBW LED strip on its Green channel and the XL enclosure fan on its
        // Red channel.
        //
        // The channels on these strips are also apparently being GRB, not RGB,
        // as the Neopixel driver expects.
        leds.Set(ColorRGBW(current_color.g, current_color.r, current_color.b).data, 0);
        leds.Set(ColorRGBW(enclosure_fan_pwm, current_color.w, 0).data, 1);
    } else {
        for (size_t i = 0; i < led_drivers_count; ++i) {
            leds.Set(ColorRGBW(current_color.g, current_color.r, current_color.b).data, i);
        }
    }

    leds.Tick();
#endif
}
