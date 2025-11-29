#pragma once

namespace leds {

/**
 * @brief A class encapsulating handling of LEDs and other peripherals
 * connected to LED interfaces, like LCD backlight and XL enclosure fan.
 *
 * Takes care of updating the LED states and animations and passing the data
 * over to the Neopixel/ws2812 interfaces.
 */
class LEDManager {
public:
    static LEDManager &instance();

    void init();

    void update();

    /**
     * @brief Called from the power panic module to quickly turn off leds from the AC fault task.
     */
    void enter_power_panic();

    /**
     * @param brighthess Brightness in percents (1-100)
     */
    void set_lcd_brightness(uint8_t brightness);

private:
    freertos::Mutex power_panic_mutex;
    bool power_panic { false };
};

}; // namespace leds
