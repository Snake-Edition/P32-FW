#pragma once

#include <enum_array.hpp>
#include <leds/color.hpp>

namespace leds {

class SimpleTransitionController {
public:
    void update();

    void set(ColorRGBW color, uint32_t transition_time);

    ColorRGBW color() const {
        return current_color;
    }

private:
    ColorRGBW target_color;
    ColorRGBW prev_color;
    ColorRGBW current_color;
    uint32_t transition_start { 0 };
    uint32_t transition_time { 0 };
    bool animation_finished { false };
};

} // namespace leds
