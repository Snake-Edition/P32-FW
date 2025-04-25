#pragma once

#include "Marlin.h"
#include <option/has_modularbed.h>

#if HAS_MODULARBED()

class AdvancedModularBed {
public:
    virtual ~AdvancedModularBed() = default;
    virtual uint16_t idx(const uint8_t column, const uint8_t row) = 0;
    virtual void set_target(const uint8_t column, const uint8_t row, float target_temp) = 0;
    virtual float get_target(const uint8_t column, const uint8_t row) = 0;
    virtual float get_temp(const uint8_t column, const uint8_t row) = 0;
    virtual void update_bedlet_temps(uint16_t enabled_mask, float target_temp) = 0;

    inline void set_gradient_cutoff(float value) {
        bedlet_gradient_cutoff = value;
    }

    void set_gradient_exponent(float value) {
        bedlet_gradient_exponent = value;
    }

    void set_expand_to_sides(bool enabled) {
        expand_to_sides_enabled = enabled;
    }

protected:
    // Bedlet temperature gradient calculation, uses this equation:
    // BEDLET_TEMP = NEAREST_ACTIVE_BEDLET_TEMPERATURE - NEAREST_ACTIVE_BEDLET_TEMPERATURE * ( 1 / bedlet_gradient_cutoff * ACTIVE_BEDLET_DISTANCE)^bedlet_gradient_exponent;
    float bedlet_gradient_cutoff = 2.0f; // Bedlet this far apart from active bedlet will have zero target temperature
    float bedlet_gradient_exponent = 2.0f; // Exponent used in equation to calculate heatbedlets temperature gradient
    bool expand_to_sides_enabled = true; // Enable expansion of heated area to sides in order to prevent warping from bed material thermal expansion
};

extern AdvancedModularBed * const advanced_modular_bed;

#endif
