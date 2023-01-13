/**
 * @file eeprom_current.hpp
 * @author Radek Vana
 * @brief current version of eeprom
 * without padding and crc since they are not imported and would not match anyway
 * @date 2022-01-17
 */

#include "eeprom_v11.hpp"
#include "footer_eeprom.hpp"

namespace eeprom::current {

#pragma once
#pragma pack(push)
#pragma pack(1)

/**
 * @brief body of eeprom v11
 * without head, padding and crc
 * eeprom setting storage has changed to using 5 bits pre item and last 3 bits are ones to force default footer setting on FW < 3.3.4
 */
struct vars_body_t : public eeprom::v11::vars_body_t {
    uint8_t EEVAR_LLAMA_EXTRUDER_TYPE;
    float EEVAR_LLAMA_EXTRUDER_ESTEPS;
    uint8_t EEVAR_LLAMA_HOTEND_FAN_SPEED;
    uint8_t EEVAR_LLAMA_SKEW_ENABLED;
    float EEVAR_LLAMA_SKEW_XY;
    float EEVAR_LLAMA_SKEW_XZ;
    float EEVAR_LLAMA_SKEW_YZ;
    uint8_t EEVAR_LLAMA_EXTRUDER_REVERSE;
};

#pragma pack(pop)

static_assert(sizeof(vars_body_t) == sizeof(eeprom::v11::vars_body_t) + 4 * sizeof(uint8_t) + 4 * sizeof(float), "eeprom body size does not match");

constexpr vars_body_t body_defaults = {
    eeprom::v11::body_defaults,
    0,     // EEVAR_LLAMA_EXTRUDER_TYPE,
    0,     // EEVAR_LLAMA_EXTRUDER_ESTEPS,
    0,     // EEVAR_LLAMA_HOTEND_FAN_SPEED,
    false, // EEVAR_LLAMA_SKEW_ENABLED,
    0,     // EEVAR_LLAMA_SKEW_XY,
    0,     // EEVAR_LLAMA_SKEW_XZ,
    0,     // EEVAR_LLAMA_SKEW_YZ,
    0,     // EEVAR_LLAMA_EXTRUDER_REVERSE,
};

inline vars_body_t convert(const eeprom::v11::vars_body_t &src) {
    vars_body_t ret = body_defaults;

    // copy entire v11 struct
    memcpy(&ret, &src, sizeof(eeprom::v11::vars_body_t));

    return ret;
}

} // namespace
