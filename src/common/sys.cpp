/// @file
#include <common/sys.hpp>

#include <common/shared_config.h>
#include <common/st25dv64k.h>
#include <stm32f4xx.h>

// firmware update flag
static const constexpr uint16_t FW_UPDATE_FLAG_ADDRESS = 0x040B;

version_t &boot_version = *(version_t *)(BOOTLOADER_VERSION_ADDRESS); // (address) from flash -> "volatile" is not necessary

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
