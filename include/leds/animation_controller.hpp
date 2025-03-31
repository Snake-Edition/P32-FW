#pragma once

#include <enum_array.hpp>
#include <leds/color.hpp>
#include <timing.h>

namespace leds {

template <template <size_t count> typename AnimationType, size_t count>
class AnimationController {
public:
    using AnimationParams = AnimationType<count>::Params;

    AnimationController(const AnimationParams &startup_params)
        : current_animation { &startup_params, startup_params }
        , prev_animation { &startup_params, startup_params } {
    }

    void update() {
        uint32_t time_ms = ticks_ms();

        data_ = current_animation.second.render();

        float xfade = static_cast<float>(time_ms - current_animation.second.get_start_time()) / transition_time_ms;
        if (xfade < 1.0f) {
            auto prev_data = prev_animation.second.render();
            for (size_t i = 0; i < count; ++i) {
                data_[i] = prev_data[i].cross_fade(data_[i], xfade);
            }
        }
    }

    void set(const AnimationParams &params) {
        if (&params != current_animation.first) {
            prev_animation = current_animation;
            current_animation.first = &params;
            current_animation.second.start(params);
        }
    }

    std::span<const ColorRGBW, count> data() const {
        return data_;
    }

private:
    using AnimationPair = std::pair<const AnimationParams *, AnimationType<count>>;

    static constexpr uint32_t transition_time_ms { 400 };

    AnimationPair current_animation;
    AnimationPair prev_animation;

    std::array<ColorRGBW, count> data_;
};

} // namespace leds
