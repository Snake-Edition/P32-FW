#include <leds/color.hpp>

namespace leds {

ColorRGBW ColorRGBW::from_hsv(ColorHSV hsv) {
    if (hsv.h > 360 || hsv.h < 0 || hsv.s > 100 || hsv.s < 0 || hsv.v > 100 || hsv.v < 0) {
        return ColorRGBW();
    }

    const float h = hsv.h;
    const float s = hsv.s / 100.0f;
    const float v = hsv.v / 100.0f;

    const auto convert = [&](uint8_t n) -> uint8_t {
        float k = fmod(n + h / 60, 6);
        float intermediate = (v * s * std::max(0.f, std::min(std::min(k, 4 - k), 1.f)));
        float e = (v - intermediate);
        return 255 * e;
    };

    return ColorRGBW(convert(5), convert(3), convert(1), 0);
}

ColorRGBW ColorRGBW::fade(float brightness) const {
    uint8_t nr = r * brightness;
    uint8_t ng = g * brightness;
    uint8_t nb = b * brightness;
    uint8_t nw = w * brightness;
    return { nr, ng, nb, nw };
}

ColorRGBW ColorRGBW::clamp(uint8_t max_level) const {
    using std::min;
    return { min(r, max_level), min(g, max_level), min(b, max_level), min(w, max_level) };
}

ColorRGBW ColorRGBW::cross_fade(const ColorRGBW &c, float ratio) const {
    float old_ratio = std::max(0.5f - ratio, 0.0f) * 2;
    float new_ratio = std::max(ratio - 0.5f, 0.0f) * 2;
    uint8_t nr = r * old_ratio + c.r * new_ratio;
    uint8_t ng = g * old_ratio + c.g * new_ratio;
    uint8_t nb = b * old_ratio + c.b * new_ratio;
    uint8_t nw = w * old_ratio + c.w * new_ratio;
    return { nr, ng, nb, nw };
}

ColorRGBW ColorRGBW::blend(const ColorRGBW &c, float ratio) const {
    float rratio = 1.0f - ratio;
    uint8_t nr = r * rratio + c.r * ratio;
    uint8_t ng = g * rratio + c.g * ratio;
    uint8_t nb = b * rratio + c.b * ratio;
    uint8_t nw = w * rratio + c.w * ratio;
    return { nr, ng, nb, nw };
}

} // namespace leds
