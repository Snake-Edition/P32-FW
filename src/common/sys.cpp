/// @file
#include <common/sys.hpp>

#include "stm32f4xx.h"
#include <buddy/main.h>
#include <common/shared_config.h>
#include <common/st25dv64k.h>
#include <cstdlib>
#include <logging/log.hpp>
#include <stm32f4xx.h>

LOG_COMPONENT_REF(Buddy);

#define DFU_REQUEST_RTC_BKP_REGISTER RTC->BKP0R

// magic value of RTC->BKP0R for requesting DFU bootloader entry
static const constexpr uint32_t DFU_REQUESTED_MAGIC_VALUE = 0xF1E2D3C5;

// firmware update flag
static const constexpr uint16_t FW_UPDATE_FLAG_ADDRESS = 0x040B;

// int sys_pll_freq = 100000000;
int sys_pll_freq = 168000000;

version_t &boot_version = *(version_t *)(BOOTLOADER_VERSION_ADDRESS); // (address) from flash -> "volatile" is not necessary

volatile uint8_t *psys_fw_valid = (uint8_t *)0x080FFFFF; // last byte in the flash

[[noreturn]] void __RAM_FUNC sys_reset() {
    // This needs to be RAM function as it is called when erasing the flash.
    // Also, we manually inline HAL_NVIC_SystemReset() here to ensure every
    // part of this function really lives in RAM.

    // this code is Cortex-M4 specific
    static_assert(__CORTEX_M == 4);

    // disable interrupts
    asm volatile("cpsid i"
                 :
                 :
                 : "memory");

    // data synchronization barrier
    asm volatile("dsb"
                 :
                 :
                 : "memory");

    // request reset
    SCB->AIRCR = (uint32_t)((0x5FAUL << SCB_AIRCR_VECTKEY_Pos) | (SCB->AIRCR & SCB_AIRCR_PRIGROUP_Msk) | SCB_AIRCR_SYSRESETREQ_Msk);

    // data synchronization barrier
    asm volatile("dsb"
                 :
                 :
                 : "memory");

    // wait for reset
    for (;;) {
        asm volatile("nop");
    }
}

bool sys_debugger_attached() {
    return CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk;
}

void sys_dfu_request_and_reset() {
    DFU_REQUEST_RTC_BKP_REGISTER = DFU_REQUESTED_MAGIC_VALUE;
    sys_reset();
}

bool sys_dfu_requested() {
    return DFU_REQUEST_RTC_BKP_REGISTER == DFU_REQUESTED_MAGIC_VALUE;
}

void sys_dfu_boot_enter() {
    // clear the flag
    DFU_REQUEST_RTC_BKP_REGISTER = 0;

    // disable systick
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

    // remap memory
    SYSCFG->MEMRMP = 0x01;

    // enter the bootloader
    volatile uintptr_t system_addr_start = 0x1FFF0000;
    auto system_bootloader_start = (void (*)(void))(*(uint32_t *)(system_addr_start + 4));
    __set_MSP(*(uint32_t *)system_addr_start); // prepare stack pointer
    system_bootloader_start(); // jump into the bootloader

    // we should never reach this
    abort();
}

int sys_calc_flash_latency(int freq) {
    if (freq < 30000000) {
        return 0;
    }
    if (freq < 60000000) {
        return 1;
    }
    if (freq < 90000000) {
        return 2;
    }
    if (freq < 12000000) {
        return 3;
    }
    if (freq < 15000000) {
        return 4;
    }
    return 5;
}

bool sys_fw_update_is_enabled() {
    return std::to_underlying(FwAutoUpdate::on) == st25dv64k_user_read(FW_UPDATE_FLAG_ADDRESS);
}

void sys_fw_update_enable() {
    st25dv64k_user_write(FW_UPDATE_FLAG_ADDRESS, std::to_underlying(FwAutoUpdate::on));
}

void sys_fw_update_disable() {
    st25dv64k_user_write(FW_UPDATE_FLAG_ADDRESS, std::to_underlying(FwAutoUpdate::off));
}

bool version_less_than(const version_t *a, const uint8_t major, const uint8_t minor, const uint8_t patch) {
    if (a->major != major) {
        return a->major < major;
    }
    if (a->minor != minor) {
        return a->minor < minor;
    }
    return a->patch < patch;
}
