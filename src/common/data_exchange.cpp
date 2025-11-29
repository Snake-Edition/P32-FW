/// @file
#include <common/data_exchange.hpp>

#include <common/otp_types.hpp>
#include <common/wdt.hpp>
#include <cstdint>
#include <cstring>
#include <device/hal.h>
#include <hw/at21csxx.hpp>
#include <option/bootloader.h>

// pin PA13 state
static constexpr uint8_t APPENDIX_FLAG_MASK = 0x01;

enum class FwAutoUpdate : uint8_t {
    on = 0xAA,
    off = 0x00,
    older = 0x55,
    specified = 0xBB,
    tester_mode_1 = 0xCA,
    tester_mode_2 = 0xCB,
    tester_mode_3 = 0xCC,
};

struct __attribute__((packed)) DataExchange {
    FwAutoUpdate fw_update_flag;
    uint8_t appendix_status;
    uint8_t fw_signature;
    uint8_t bootloader_valid; //< Initialized by preboot to false; bootloader sets it to true
    char bbf_sfn[13];
    uint8_t reserved__[3]; // aligned to N*4B (20B), set to 0 - so value is defined, in case we need ot gor versioning etc.
    OtpStatus xlcd_status; // 8B
    XlcdEeprom xlcd_eeprom; // 32B
    OtpStatus loveboard_status; // 8B
    LoveBoardEeprom loveboard_eeprom; // 32B
}; // 100B in total

DataExchange ram_data_exchange __attribute__((section(".boot_fw_data_exchange")));

#if !BOOTLOADER()

/**
 * @brief reads OTP data from external onewire AT21CSxx eeprom
 * currently supports only OTP_v2 (and OTP_v5 since it is the same)
 * supporting obsolete OTP_v0, OTP_v1 and OTP_v3 does not make sense
 * and OTP_v4 is the same as OTP_v2/OTP_v5 with extra mac address
 */
static void load_otp_from_eeprom(AT21CSxx &eeprom, OTP_v2 &calib_data, OtpStatus &status) {
    // Read data from EEPROM
    wdt_iwdg_refresh();
    status.data_valid = false;
    calib_data = {};

    constexpr const int MAX_RETRY = 8;
    for (status.retried = 0; status.retried < MAX_RETRY; ++status.retried) {
        if (eeprom.read_block(0, (uint8_t *)&calib_data, sizeof(calib_data)) != 1) {
            wdt_iwdg_refresh();
            continue;
        }

        if (calib_data.size == sizeof(calib_data) && calib_data.datamatrix[0] == '0' && calib_data.datamatrix[1] == '0') { // heuristics: if read failed somehow, these values are probably not OK
            status.data_valid = true;
            break;
        } else {
            wdt_iwdg_refresh();
            eeprom.reset_discovery();
        }
    }

    status.single_read_error_counter = eeprom.get_single_read_error();
    status.repeated_read_error_counter = eeprom.get_repeated_read_error();
    status.cyclic_read_error_counter = eeprom.get_cyclic_read_error();
}

    #if HAS_XLCD()
static void read_xlcd(XlcdEeprom &data, OtpStatus &status) {
    // LCD reset
    __HAL_RCC_GPIOG_CLK_ENABLE(); // enable lcd reset pin port clock
    GPIO_InitTypeDef GPIO_InitStruct {};
    GPIO_InitStruct.Pin = GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_WritePin(GPIOG, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

    __HAL_RCC_GPIOC_CLK_ENABLE(); // enable lcd eeprom pin port clock
    AT21CSxx eeprom { GPIOC, GPIO_PIN_8 };
    load_otp_from_eeprom(eeprom, data, status);
}
    #endif

    #if HAS_LOVE_BOARD() || PRINTER_IS_PRUSA_MK3_5()
static void read_loveboard(LoveBoardEeprom &data, OtpStatus &status) {
    __HAL_RCC_GPIOF_CLK_ENABLE(); // enable loveboard eeprom pin port clock
    AT21CSxx eeprom { GPIOF, GPIO_PIN_13 };
    load_otp_from_eeprom(eeprom, data, status);
}
    #endif

void data_exchange_init() {
    ram_data_exchange.fw_update_flag = FwAutoUpdate::off;
    ram_data_exchange.appendix_status = 0; // the state is actually unknown, this represents no appendix
    ram_data_exchange.fw_signature = 0;
    ram_data_exchange.bootloader_valid = 0;
    ram_data_exchange.bbf_sfn[0] = '\0';

    ram_data_exchange.xlcd_status = {};
    ram_data_exchange.xlcd_eeprom = {};
    ram_data_exchange.loveboard_status = {};
    ram_data_exchange.loveboard_eeprom = {};

    #if HAS_XLCD()
    read_xlcd(ram_data_exchange.xlcd_eeprom, ram_data_exchange.xlcd_status);
    #endif

    // MK3.5 doesn't have a loveboard, but it needs the detection to complain if it's running on an MK4
    #if HAS_LOVE_BOARD() || PRINTER_IS_PRUSA_MK3_5()
    read_loveboard(ram_data_exchange.loveboard_eeprom, ram_data_exchange.loveboard_status);
    #endif
}
#else
void data_exchange_init() {}
#endif

static FwAutoUpdate get_auto_update_flag(void) {
    switch (ram_data_exchange.fw_update_flag) {
    case FwAutoUpdate::on:
    case FwAutoUpdate::off:
    case FwAutoUpdate::older:
    case FwAutoUpdate::specified:
    case FwAutoUpdate::tester_mode_1:
    case FwAutoUpdate::tester_mode_2:
    case FwAutoUpdate::tester_mode_3:
        return ram_data_exchange.fw_update_flag;
    }
    return FwAutoUpdate::off; // somehow corrupted data in shared RAM, no update
}

namespace data_exchange {

#if HAS_XLCD()
OtpStatus get_xlcd_status() {
    return ram_data_exchange.xlcd_status;
}

XlcdEeprom get_xlcd_eeprom() {
    return ram_data_exchange.xlcd_eeprom;
}
#endif

#if HAS_LOVE_BOARD() || PRINTER_IS_PRUSA_MK3_5()
OtpStatus get_loveboard_status() {
    return ram_data_exchange.loveboard_status;
}

LoveBoardEeprom get_loveboard_eeprom() {
    return ram_data_exchange.loveboard_eeprom;
}
#endif

bool has_fw_signature() {
    return ram_data_exchange.fw_signature != 0;
}

void fw_update_on_restart_enable() {
    ram_data_exchange.fw_update_flag = FwAutoUpdate::on;
}

void fw_update_older_on_restart_enable() {
    ram_data_exchange.fw_update_flag = FwAutoUpdate::older;
}

bool is_bootloader_valid() {
    return ram_data_exchange.bootloader_valid ? 1 : 0;
}

void set_reflash_bbf_sfn(const char *sfn) {
    ram_data_exchange.fw_update_flag = FwAutoUpdate::specified;
    strlcpy((char *)ram_data_exchange.bbf_sfn, sfn, sizeof(DataExchange::bbf_sfn));
}

bool has_apendix() {
    return ram_data_exchange.appendix_status & APPENDIX_FLAG_MASK;
}
} // namespace data_exchange

bool running_in_tester_mode() {
    switch (get_auto_update_flag()) {
    case FwAutoUpdate::on:
    case FwAutoUpdate::off:
    case FwAutoUpdate::older:
    case FwAutoUpdate::specified:
        break;
    case FwAutoUpdate::tester_mode_1:
    case FwAutoUpdate::tester_mode_2:
    case FwAutoUpdate::tester_mode_3:
        return true;
    }
    return false;
}
