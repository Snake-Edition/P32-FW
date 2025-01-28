#pragma once

#include <cstdint>
#include <module/planner.h>

namespace pressure_advance {

struct Config {
    float pressure_advance = 0.f;
    float smooth_time = 0.04f;

    bool operator==(const Config &rhs) const = default;
};

// pressure advance initialization
void init();

// configure e axis
void set_axis_e_config(const Config &config);
const Config &get_axis_e_config();

// Guard to globally disable PA
class PressureAdvanceDisabler {
    static inline unsigned nesting = 0;

    // Original PA settings
    static inline Config config_orig;

public:
    [[nodiscard]] PressureAdvanceDisabler() {
        if (nesting == 0) {
            config_orig = get_axis_e_config();
            if (config_orig.pressure_advance > 0.f) {
                planner.synchronize();
                set_axis_e_config({ .pressure_advance = 0.f });
            }
        }
        ++nesting;
    }

    ~PressureAdvanceDisabler() {
        if (--nesting == 0) {
            if (config_orig != get_axis_e_config()) {
                planner.synchronize();
                set_axis_e_config(config_orig);
            }
        }
    }

    static bool is_active() {
        return nesting > 0;
    }
};

} // namespace pressure_advance
