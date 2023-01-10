/*
 * Copyright (c) 2021 Matthew Lloyd <github@matthewlloyd.net>
 * All rights reserved.
 *
 * This file is part of Llama Mini.
 *
 * Llama Mini is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Llama Mini is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Llama Mini. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <stdbool.h>
#include "variant8.h"
#include "eeprom.h"

// Llama eeprom datasize must always increase with increasing version.
// Do not remove fields.

// Llama eeprom map
static const eeprom_entry_t eeprom_llama_map[] = {
    { "VERSION", VARIANT8_UI16, 1, 0 },        // EEVAR_LLAMA_VERSION
    { "DATASIZE", VARIANT8_UI16, 1, 0 },       // EEVAR_LLAMA_DATASIZE
    { "EXTRUDER_TYPE", VARIANT8_UI8, 1, 0 },   // EEVAR_LLAMA_EXTRUDER_TYPE
    { "EXTRUDER_ESTEPS", VARIANT8_FLT, 1, 0 }, // EEVAR_LLAMA_EXTRUDER_ESTEPS
    { "HOTEND_FAN_SPD", VARIANT8_UI8, 1, 0 },  // EEVAR_LLAMA_HOTEND_FAN_SPEED
    { "SKEW_ENABLED", VARIANT8_UI8, 1, 0 },    // EEVAR_LLAMA_SKEW_ENABLED
    { "SKEW_XY", VARIANT8_FLT, 1, 0 },         // EEVAR_LLAMA_SKEW_XY
    { "SKEW_XZ", VARIANT8_FLT, 1, 0 },         // EEVAR_LLAMA_SKEW_XZ
    { "SKEW_YZ", VARIANT8_FLT, 1, 0 },         // EEVAR_LLAMA_SKEW_YZ
    { "EXTRUDER_REV", VARIANT8_UI8, 1, 0 },    // EEVAR_LLAMA_EXTRUDER_REVERSE
    { "CRC32", VARIANT8_UI32, 1, 0 },          // EEVAR_LLAMA_CRC32
};

enum {
    EEPROM_LLAMA_ADDRESS = EEPROM_ADDRESS + EEPROM_MAX_DATASIZE_H - 50, // continue after main EEPROM
    EEPROM_LLAMA_VERSION = 3
};

enum {
    EEVAR_LLAMA_VERSION,  // uint16_t eeprom version
    EEVAR_LLAMA_DATASIZE, // uint16_t eeprom datasize
    EEVAR_LLAMA_EXTRUDER_TYPE,
    EEVAR_LLAMA_EXTRUDER_ESTEPS,
    EEVAR_LLAMA_HOTEND_FAN_SPEED,
    EEVAR_LLAMA_SKEW_ENABLED,
    EEVAR_LLAMA_SKEW_XY,
    EEVAR_LLAMA_SKEW_XZ,
    EEVAR_LLAMA_SKEW_YZ,
    EEVAR_LLAMA_EXTRUDER_REVERSE,
    EEVAR_LLAMA_CRC32, // uint32_t crc32
};

#pragma pack(push, 1)

typedef struct _eeprom_llama_vars_t {
    uint32_t CRC32;
    uint16_t VERSION;
    uint16_t DATASIZE;
    uint8_t EXTRUDER_TYPE;
    float EXTRUDER_ESTEPS;
    uint8_t HOTEND_FAN_SPEED;
    uint8_t SKEW_ENABLED;
    float SKEW_XY;
    float SKEW_XZ;
    float SKEW_YZ;
    uint8_t EXTRUDER_REVERSE;
} eeprom_llama_vars_t;

#pragma pack(pop)

#define EEPROM_LLAMA_MIN_DATASIZE (28)
static const constexpr uint32_t EEPROM_LLAMA_VARCOUNT = sizeof(eeprom_llama_map) / sizeof(eeprom_entry_t);
static const constexpr uint32_t EEPROM_LLAMA_DATASIZE = sizeof(eeprom_llama_vars_t);
static_assert(EEPROM_LLAMA_DATASIZE >= EEPROM_LLAMA_MIN_DATASIZE, "EEPROM_LLAMA_DATASIZE too small. Only add fields.");

static_assert(EEPROM_DATASIZE_H + EEPROM_LLAMA_DATASIZE <= EEPROM_MAX_DATASIZE_H, "Not enough space for EEPROM variables.");
static_assert(EEPROM_LLAMA_ADDRESS >= EEPROM_ADDRESS + EEPROM_DATASIZE_H, "Overlapping space. Move EEPROM_LLAMA_ADDRESS.");
static_assert(EEPROM_LLAMA_ADDRESS + EEPROM_LLAMA_DATASIZE > EEPROM_ADDRESS + EEPROM_DATASIZE_H, "EEPROM (partially) out of range. Move EEPROM_LLAMA_ADDRESS.");

// Llama eeprom variable defaults
static const eeprom_llama_vars_t eeprom_llama_var_defaults = {
    0xffffffff,            // EEVAR_LLAMA_CRC32
    EEPROM_LLAMA_VERSION,  // EEPROM_LLAMA_VERSION
    EEPROM_LLAMA_DATASIZE, // EEPROM_LLAMA_DATASIZE
    0,                     // EEPROM_LLAMA_EXTRUDER_TYPE
    325.f,                 // EEPROM_LLAMA_EXTRUDER_ESTEPS
    0,                     // EEPROM_LLAMA_HOTEND_FAN_SPEED
    0,                     // EEPROM_LLAMA_SKEW_ENABLED
    0.f,                   // EEPROM_LLAMA_SKEW_XY
    0.f,                   // EEPROM_LLAMA_SKEW_XZ
    0.f,                   // EEPROM_LLAMA_SKEW_YZ
    0,                     // EEPROM_LLAMA_EXTRUDER_REVERSE
};

extern uint8_t eeprom_llama_get_var_count(void);
// equivalent functions for Llama
extern void eeprom_llama_init(void);
extern void eeprom_llama_defaults(void);
extern variant8_t eeprom_llama_get_var(uint8_t id);
extern void eeprom_llama_set_var(uint8_t id, variant8_t var, bool update_crc32 = true);

void llama_apply_fan_settings();
void llama_apply_skew_settings();
