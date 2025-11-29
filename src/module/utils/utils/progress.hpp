/// \file
#pragma once

#include <cstdint>
#include <cassert>
#include <algorithm>

using ProgressPercent = uint8_t;

struct ProgressSpan {
    ProgressPercent min = 0;
    ProgressPercent max = 100;

    constexpr ProgressPercent map(float normalized_progress) const {
        assert(normalized_progress >= 0 && normalized_progress <= 1);
        return min + normalized_progress * (max - min);
    }

    constexpr bool operator==(const ProgressSpan &o) const = default;
};

/// Maps \param value from range \param min - \param max to range [0, 1]
inline float to_normalized_progress(const float min, const float max, const float value) {
    // Handle edge case where min >= max (avoid division by zero)
    const float range = max - min;
    if (range <= 0.0f) {
        return 1.0f;
    }

    return std::clamp((value - min) / range, 0.0f, 1.0f);
}
