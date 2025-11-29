// config.h - main configuration file
#pragma once

#include "stdint.h"

// bootloader version
static const uint32_t BOOTLOADER_VERSION_ADDRESS = 0x0801FFFA;

struct __attribute__((packed)) version_t {
    uint8_t major;
    uint8_t minor;
    uint8_t patch;
};
