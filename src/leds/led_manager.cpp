#include "leds/led_manager.hpp"
#include "display.hpp"
#include "led_lcd_cs_selector.hpp"

#include <leds/status_leds_handler.hpp>
#include "neopixel.hpp"
#include <option/has_side_leds.h>

#include <config_store/store_instance.hpp>

#if HAS_SIDE_LEDS()
    #include "leds/side_strip_control.hpp"
#endif

#include <option/has_door_sensor.h>
#if HAS_DOOR_SENSOR()
    #include <buddy/door_sensor.hpp>
#endif

#include <option/has_xbuddy_extension.h>
#if HAS_XBUDDY_EXTENSION()
    #include <feature/xbuddy_extension/xbuddy_extension.hpp>
#endif

extern osThreadId displayTaskHandle;

using StatusLeds = neopixel::LedsSPI10M5Hz<4, GuiLedsWriter::write>;

static StatusLeds &get_status_leds() {
    static StatusLeds ret;
    return ret;
}

namespace leds {

LEDManager &LEDManager::instance() {
    static LEDManager instance;
    return instance;
}

void LEDManager::init() {
    // update the LEDs in init to turn them off (in case they were set to a color before a reset)
    // except the LCD backlight, set that to 100% brightness
    set_lcd_brightness(100);
    get_status_leds().update();

#if HAS_SIDE_LEDS()
    side_strip_control.set_max_brightness(config_store().side_leds_max_brightness.get());
    side_strip_control.set_dimming_enabled(config_store().side_leds_dimming_enabled.get());
#endif
}

void LEDManager::update() {
    std::lock_guard lock(power_panic_mutex);
    if (power_panic) {
        return;
    }

    auto &status_leds_handler = leds::StatusLedsHandler::instance();
    status_leds_handler.update();
    auto data = status_leds_handler.led_data();

    auto &status_leds = get_status_leds();
    for (uint8_t i = 0; i < data.size(); ++i) {
        status_leds.set(data[i].data, i);
    }

#if HAS_XBUDDY_EXTENSION()
    // Bed LEDs copy LCD status bar strip
    buddy::xbuddy_extension().set_bed_leds_color(data[1].data);
#endif

    status_leds.update();

#if HAS_SIDE_LEDS()
    #if HAS_DOOR_SENSOR()
    if (buddy::door_sensor().state() == buddy::DoorSensor::State::door_open) {
        side_strip_control.ActivityPing();
    }
    #endif
    side_strip_control.Tick();
#endif
}

void LEDManager::enter_power_panic() {
    {
        std::lock_guard lock(power_panic_mutex);
        power_panic = true;
    }

    // normally, GUI is accessing LCD & LEDs SPIs, but this is called from the
    // task handling power panic, and we need to turn the leds off quickly. So
    // we'll steal the display's SPI in a hacky way.

    // 1. configure led animations to off, in case gui would want to write them again, this lock mutex, so has to be done before suspending display task
#if HAS_SIDE_LEDS()
    side_strip_control.PanicOff();
#endif

    // 2. Temporary suspend display task, so that it doesn't interfere with turning off leds
    osThreadSuspend(displayTaskHandle);

    // 3. Safe mode for display SPI is enabled (that disables DMA transfers and writes directly to SPI)
    display::enable_safe_mode();

    // 4. Reinitialize SPI, so that we terminate any ongoing transfers to display or leds
    // 5. turn off actual leds
#if BOARD_IS_XLBUDDY()
    hw_init_spi_side_leds();
#endif

    SPI_INIT(lcd);
    auto &status_leds = get_status_leds();
    status_leds.set_all(0x0);
    status_leds.update();

#if HAS_SIDE_LEDS()
    side_strip.SetColor(ColorRGBW());
    side_strip.Update();
#endif

    // 5. reenable display task
    osThreadResume(displayTaskHandle);
}

void LEDManager::set_lcd_brightness(uint8_t brightness) {
    // LCD backlight is connected to the green channel of the fourth LED (index 3) on the status strip
    get_status_leds().set(ColorRGBW(0, (std::min<uint8_t>(brightness, 100) * 255) / 100, 0).data, 3);
}

} // namespace leds
