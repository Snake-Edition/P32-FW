#pragma once
#include <inttypes.h>
#include <tuple>
#include <optional>
#include <inplace_function.hpp>
#include <option/bootloader_update.h>

namespace buddy::bootloader {

struct Version {
    uint8_t major;
    uint8_t minor;
    uint8_t patch;
} __attribute__((packed));

/// Return version of installed bootloader.
Version get_version();

// Bootloader sectors
static constexpr const size_t bootloader_sector_sizes[] = { 16384, 16384, 16384, 16384, 65536 };
static constexpr const size_t bootloader_sector_count = std::size(bootloader_sector_sizes);

#if BOOTLOADER_UPDATE()

/// @returns true if the bootloader needs to be updated/reflashed
/// @param required minimal version
bool needs_update(Version required);

/// @returns true if the bootloader needs to be updated/reflashed
bool needs_update();

/// @returns true if the pre-boot needs to be updated/reflashed
bool preboot_needs_update();

enum class UpdateStage {
    LookingForBbf,
    PreparingUpdate,
    Updating,
};

using ProgressHook = stdext::inplace_function<void(int percent_done, UpdateStage stage)>;

/// Update the bootloader (if needed together with preboot) on the MCU
void update(ProgressHook progress);

#endif

/**
 * @brief Erase part of firmware, so it cannot run and needs to be reflashed by bootloader.
 * @return false on error, does not return on success
 */
[[nodiscard]] bool fw_invalidate(void);

}; // namespace buddy::bootloader
