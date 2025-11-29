#pragma once

namespace leds {

enum class DimmingEnabled : uint8_t {
    never = 0, // Dimming is disabled -> Leds are always on
    always = 1, // Dimming is always enabled -> Leds will dimm always after idle timeout
    not_printing = 2, // Dimming is enabled only when not printing -> Leds will dimm after idle timeout only when not printing
    _cnt = 3
};

} // namespace leds
