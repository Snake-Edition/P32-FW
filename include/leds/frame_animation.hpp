#pragma once

#include <timing.h>
#include <leds/color.hpp>

namespace leds {

template <size_t count>
class FrameAnimation {
public:
    using Frame = std::array<uint8_t, count>;

    // frame_length: The time for which the given frame is displayed.
    //
    // frame_delay: The delay between two frames. If zero, frames follow
    // immediately one after another.
    // This means the period of one frame is frame_length + frame_delay
    //
    // blend_time: The time interval in which the current frame blends out and
    // also the new frame blends in. If frame_delay is 0, the new frame starts
    // blending in at the same time the old one is blending out. If frame_delay
    // is greater than 0, the new frame starts blending in later, if
    // frame_delay is greater than blend_time, there is a short period of LEDs
    // being dark beween current frame blending out and the new one blending
    // in.
    //
    // All times are in ms.
    struct Params {
        ColorRGBW color;
        uint32_t frame_length { 1000 };
        uint32_t frame_delay { 0 };
        uint32_t blend_time { 300 };
        std::span<const Frame> frames;
    };

    FrameAnimation(const Params &default_params)
        : params(&default_params) {}

    void start(const Params &p) {
        params = &p;
        start_time = ticks_ms();
    }

    std::array<ColorRGBW, count> render() const {
        std::array<ColorRGBW, count> data;

        // single frame optimization - nothing to animate
        if (params->frames.size() == 1) {
            for (size_t i = 0; i < count; ++i) {
                data[i] = params->color.fade(params->frames[0][i]);
            }
            return data;
        }

        uint32_t progress_time = ticks_ms() - start_time;
        uint32_t frame_cycle_time = params->frame_length + params->frame_delay;
        uint32_t time_in_cycle = progress_time % (frame_cycle_time * params->frames.size());

        uint32_t frame_time = time_in_cycle % frame_cycle_time;
        size_t current_frame_i = time_in_cycle / frame_cycle_time;
        size_t last_frame_i = (current_frame_i > 0 ? current_frame_i : params->frames.size()) - 1;

        const uint8_t *frame = params->frames[current_frame_i].data();
        const uint8_t *last_frame;
        if (progress_time > frame_cycle_time) {
            last_frame = params->frames[last_frame_i].data();
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
            float blended_brightness = (frame[i] * blend_factor + last_frame[i] * old_blend_factor) / 100.0f;
            data[i] = params->color.fade(blended_brightness);
        }

        return data;
    }

    uint32_t get_start_time() const {
        return start_time;
    }

private:
    static constexpr uint8_t black_frame[count] = { 0 };
    static constexpr uint8_t solid_frame[count] = { 100 };

    uint32_t start_time { 0 };
    const Params *params;
};

} // namespace leds
