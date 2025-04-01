/// @file
#pragma once

[[noreturn]] void sys_reset();

bool sys_debugger_attached();

bool sys_fw_update_is_enabled();

void sys_fw_update_enable();

void sys_fw_update_disable();
