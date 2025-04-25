#pragma once

#include <option/has_modular_bed.h>
#include <printers.h>

#if HAS_MODULAR_BED()

#if PRINTER_IS_PRUSA_XL()
    #define X_HBL_COUNT 4   // Number of heatbedlets in X direction
    #define Y_HBL_COUNT 4   // Number of heatbedlets in Y direction
#elif PRINTER_IS_PRUSA_XL_DEV_KIT()
    #define X_HBL_COUNT 4   // Number of heatbedlets in X direction
    #define Y_HBL_COUNT 4   // Number of heatbedlets in Y direction
#elif PRINTER_IS_PRUSA_iX()
    #define X_HBL_COUNT 3   // Number of heatbedlets in X direction
    #define Y_HBL_COUNT 3   // Number of heatbedlets in Y direction
#else
    #error
#endif
#define X_HBL_SIZE  90  // Size of single heatbedlet in X direction including gap between heatbedlets(mm)
#define Y_HBL_SIZE  90  // Size of single heatbedlet in Y direction including gap between heatbedlets(mm)

/// Heatbed composed of multiple smaller heatbedlets.
/// Allows separate temperature controll of individual heatbedlets(HBL).
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
