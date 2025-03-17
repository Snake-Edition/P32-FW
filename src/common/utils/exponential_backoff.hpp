#pragma once

#include <algorithm>
#include <optional>

namespace buddy {

template <class T, T base, T max>
class ExponentialBackoff {
private:
    static_assert(base < max);
    std::optional<T> value = std::nullopt;

public:
    std::optional<T> get() const {
        return value;
    }
    void fail() {
        if (value.has_value()) {
            value = std::min(max, *value * 2);
        } else {
            value = base;
        }
    }
    void reset() {
        value.reset();
    }
};

} // namespace buddy
