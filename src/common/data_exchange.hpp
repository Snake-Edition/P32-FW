/// @file
#pragma once

#include <common/otp_types.hpp>
#include <option/has_xlcd.h>
#include <option/has_love_board.h>

/**
 * Init the data_exchange struct in case of no-bootloader build.
 */
void data_exchange_init();

/// @return true if firmware is running in tester mode
bool running_in_tester_mode();

namespace data_exchange {

#if HAS_XLCD()
OtpStatus get_xlcd_status();

XlcdEeprom get_xlcd_eeprom();
#endif

// MK3.5 doesn't have a loveboard, but it needs the detection to complain if it's running on an MK4
#if HAS_LOVE_BOARD() || PRINTER_IS_PRUSA_MK3_5()
OtpStatus get_loveboard_status();

LoveBoardEeprom get_loveboard_eeprom();
#endif

void fw_update_on_restart_enable();

void fw_update_older_on_restart_enable();

/// Return true if the preboot set the bootloader_valid flag to true
///
/// Warning: requires bootloader version 2.0.0 (which includes preboot)
/// or newer
bool is_bootloader_valid();

void set_reflash_bbf_sfn(const char *sfn);

bool has_apendix();

bool has_fw_signature();
} // namespace data_exchange
