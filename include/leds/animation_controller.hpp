#pragma once

#include <enum_array.hpp>
#include <leds/color.hpp>

namespace leds {

template <template <size_t count> typename AnimationType, typename AnimationEnum, size_t count>
class AnimationController {
public:
    using Mapping = EnumArray<AnimationEnum, typename AnimationType<count>::Params, static_cast<int>(AnimationEnum::_count) + 1>;

    AnimationController(const Mapping &anim_mapping, AnimationEnum startup_type)
        : animation_mapping(anim_mapping)
        , current_animation { startup_type, anim_mapping[startup_type] }
        , prev_animation { startup_type, anim_mapping[startup_type] } {
    }

    void update() {
        uint32_t time_ms = ticks_ms();

        auto new_data = current_animation.second.render();

        float xfade = static_cast<float>(time_ms - current_animation.second.get_start_time()) / transition_time;
        if (xfade < 1.0f) {
            auto prev_data = prev_animation.second.render();
            for (size_t i = 0; i < count; ++i) {
                new_data[i] = prev_data[i].cross_fade(new_data[i], xfade);
            }
        }

        for (size_t i = 0; i < count; ++i) {
            data_[i] = new_data[i];
        }
    }

    void set(AnimationEnum type) {
        if (type != current_animation.first) {
            prev_animation = current_animation;
            current_animation.first = type;
            current_animation.second.start(animation_mapping[type]);
        }
    }

    std::span<ColorRGBW> data() {
        return data_;
    }

private:
    using AnimPair = std::pair<AnimationEnum, AnimationType<count>>;

    static constexpr uint32_t transition_time { 400 };

    const Mapping &animation_mapping;
    AnimPair current_animation;
    AnimPair prev_animation;

    std::array<ColorRGBW, count> data_;
};

} // namespace leds
