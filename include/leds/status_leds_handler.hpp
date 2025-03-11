#pragma once

#include <leds/color.hpp>

#include <freertos/mutex.hpp>

namespace leds {

enum class StateAnimation : uint8_t {
    Idle,
    Printing,
    Aborting,
    Finishing,
    Warning,
    PowerPanic,
    PowerUp,
    Error,
    Custom,
    _last = Custom,
};

enum class AnimationType : uint8_t {
    Solid,
    Pulsing,
    _last = Pulsing,
};

class StatusLedsHandler {
public:
    static StatusLedsHandler &instance();

    /**
     * @brief Sets an error state, overriding whatever printer state is fetched from Marlin.
     *
     * There's currently no way of returning to a normal state (it's used for BSOD/RSOD).
     */
    void set_error();

    /**
     * @brief Sets current animation, lasting until the next state change.
     */
    void set_animation(StateAnimation state);

    /**
     * @brief Sets a custom animation, lasting until the next state change.
     */
    void set_custom_animation(const ColorRGBW &color, AnimationType type, uint16_t period_ms);

    bool get_active();
    void set_active(bool val);

    void update();

    std::span<const ColorRGBW, 3> led_data();

private:
    freertos::Mutex mutex;
    bool active { config_store().run_leds.get() };
    StateAnimation old_state { StateAnimation::Idle };
    bool is_error_state { false };
};

} // namespace leds
