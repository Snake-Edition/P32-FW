/// @file
#pragma once

#include <common/shared_config.h>
#include <cstdint>

extern version_t &boot_version; // (address) from flash -> "volatile" is not necessary

[[noreturn]] void sys_reset();

bool sys_debugger_attached();

bool sys_fw_update_is_enabled();

void sys_fw_update_enable();

void sys_fw_update_disable();

/// @return true if version a < (major, minor, patch)
bool version_less_than(const version_t *a, const uint8_t major, const uint8_t minor, const uint8_t patch);
