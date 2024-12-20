#pragma once

#include <memory>
#include <variant>

struct PrintStatusMessageDataCustom {
    /// Should be an immutable string - once it's passed to the subsystem, it cannot be changed
    std::shared_ptr<const char[]> message;

    // Comparing addresses is enough - we're comparing identicality, not equality
    bool operator==(const PrintStatusMessageDataCustom &) const = default;
};

/// Universal progress report.
/// Note: We can use (current - initial_current) / (target - initial_current) to map this to actuall percentual progress,
/// assuming the client stores first current reported by the status message.
struct PrintStatusMessageDataProgress {
    /// Current amount of the property we're progressing.
    /// The unit can be whatever - percent, °C, mm, ...
    float current = 0;

    /// Target value we are aiming for
    float target = 0;

    bool operator==(const PrintStatusMessageDataProgress &) const = default;
};

using PrintStatusMessageData = std::variant<std::monostate, PrintStatusMessageDataCustom, PrintStatusMessageDataProgress>;
