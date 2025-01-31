#pragma once

#include <leds/color.hpp>

namespace leds {

template <size_t count>
class FrameAnimation {
public:
    struct Params {
        ColorRGBW color;
        uint32_t frame_length;
        uint32_t frame_delay;
        uint32_t blend_time;
        const uint8_t (*frames)[count];
        size_t frame_count;
    };

    FrameAnimation(const Params &default_params)
        : params(&default_params) {}

    void start(const Params &p) {
        params = &p;
        start_time = ticks_ms();
    }

    std::array<ColorRGBW, count> render() {
        std::array<ColorRGBW, count> data;

        // single frame optimization - nothing to animate
        if (params->frame_count == 1) {
            for (size_t i = 0; i < count; ++i) {
                data[i] = params->color.fade(params->frames[0][i]);
            }
            return data;
        }

        uint32_t progress_time = ticks_ms() - start_time;
        uint32_t frame_cycle_time = params->frame_length + params->frame_delay;
        uint32_t time_in_cycle = progress_time % (frame_cycle_time * params->frame_count);

        uint32_t frame_time = time_in_cycle % frame_cycle_time;
        size_t current_frame_i = time_in_cycle / frame_cycle_time;
        size_t last_frame_i = (current_frame_i > 0 ? current_frame_i : params->frame_count) - 1;

        const uint8_t *frame = params->frames[current_frame_i];
        const uint8_t *last_frame;
        if (progress_time > frame_cycle_time) {
            last_frame = params->frames[last_frame_i];
        } else {
            last_frame = black_frame;
        }

        float blend_factor = 0;
        if (frame_time <= params->frame_length) {
            blend_factor = std::clamp(static_cast<float>(frame_time) / params->blend_time, 0.0f, 1.0f);
        } else {
            blend_factor = 1.0f - std::clamp(static_cast<float>(frame_time - params->frame_length) / params->blend_time, 0.0f, 1.0f);
        }

        float old_blend_factor = 1.0f - std::clamp(static_cast<float>(frame_time + params->frame_delay) / params->blend_time, 0.0f, 1.0f);
        for (size_t i = 0; i < count; ++i) {
            float fbr = frame[i] / 100.0f;
            float lfbr = last_frame[i] / 100.0f;
            float blended_brightness = fbr * blend_factor + lfbr * old_blend_factor;

            data[i] = params->color.fade(blended_brightness);
        }

        return data;
    }

    uint32_t get_start_time() {
        return start_time;
    }

private:
    static constexpr uint8_t black_frame[count] = { 0 };

    uint32_t start_time { 0 };
    const Params *params;
};

} // namespace leds
