///
/// \file
///
/// Driver for the W25xxx family of SPI flash memories.
///
/// The driver is split into two parts.
/// The `w25x_communication.hpp/cpp` handles communication with the chip and this part is considered private to the w25x module.
/// The `w25x.hpp/cpp` handles the high-level operations with the chip (read, erase, write, etc)
///
#pragma once

#include <cstdint>
#include <device/board.h>

#define W25X_BLOCK_SIZE         4096
#define W25X_BLOCK64_SIZE       0x10000
#define W25X_DUMP_START_ADDRESS 0
#if BOARD_IS_BUDDY()
    // Some MINIes have 1MB flash, some have 8M
    // 49 = 196KiB offset for crash dump
    #define W25X_ERR_START_ADDRESS (49 * W25X_BLOCK_SIZE)
    #define W25X_PP_START_ADDRESS  (50 * W25X_BLOCK_SIZE)
    #define W25X_FS_START_ADDRESS  (51 * W25X_BLOCK_SIZE)
#elif BOARD_IS_XBUDDY()
    // 8M = 2K of 4K blocks
    // 65 = 260KiB offset for crash dump
    #define W25X_ERR_START_ADDRESS (65 * W25X_BLOCK_SIZE)
    #define W25X_PP_START_ADDRESS  (66 * W25X_BLOCK_SIZE)
    #define W25X_FS_START_ADDRESS  (67 * W25X_BLOCK_SIZE)
#elif BOARD_IS_XLBUDDY()
    // 8M = 2K of 4K blocks
    // 65 = 260KiB offset for crash dump, which is the total RAM size
    #define W25X_ERR_START_ADDRESS (65 * W25X_BLOCK_SIZE)
    #define W25X_PP_START_ADDRESS  (66 * W25X_BLOCK_SIZE)
    #define W25X_FS_START_ADDRESS  (68 * W25X_BLOCK_SIZE)
#else
    #error "Unsupported board type"
#endif

inline constexpr uint32_t w25x_block_size = W25X_BLOCK_SIZE;
inline constexpr uint32_t w25x_block64_size = W25X_BLOCK64_SIZE;
inline constexpr uint32_t w25x_dump_start_address = W25X_DUMP_START_ADDRESS;
inline constexpr uint32_t w25x_error_start_adress = W25X_ERR_START_ADDRESS;
inline constexpr uint32_t w25x_pp_start_address = W25X_PP_START_ADDRESS;
inline constexpr uint32_t w25x_fs_start_address = W25X_FS_START_ADDRESS;
inline constexpr size_t w25x_pp_size = w25x_fs_start_address - w25x_pp_start_address;
inline constexpr uint32_t w25x_dump_size = w25x_error_start_adress - w25x_dump_start_address;

/// Initialize the w25x module while the scheduler is running.
///
/// When w25x is initialized using this function, all its
/// interface function must be called in task context only.
void w25x_init();

/// (Re)initialize the w25x module after the scheduler has been stopped.
///
/// Call this once to abort ongoing transfers and take over the chip.
/// This should only be used to write crash dump to external flash.
///
/// All interface functions can be called in any context but are not reentrant.
/// If w25x is reinitialized during DMA transfer it is aborted. If some
/// data is already transfered to the chip at that point those data are
/// written gracefully. If erase operation is ongoing it is completed
/// during reinitialization.
///
/// Worst case runtime is 100 seconds if called just after chip erase
/// operation has been started. Worst case runtime is 200 seconds for
/// maliciously crafted w25x responses.
///
/// @retval true on success
/// @retval false otherwise.
bool w25x_reinit_before_crash_dump();

/// Return the number of available sectors
uint32_t w25x_get_sector_count();

/// Read data from the flash.
/// Errors can be checked (and cleared) using w25x_fetch_error()
void w25x_rd_data(uint32_t addr, uint8_t *data, uint16_t cnt);

/// Write data to the flash (the sector has to be erased first)
/// Errors can be checked (and cleared) using w25x_fetch_error()
void w25x_program(uint32_t addr, const uint8_t *data, uint32_t cnt);

/// Erase single sector of the flash
/// Errors can be checked (and cleared) using w25x_fetch_error()
void w25x_sector_erase(uint32_t addr);

/// Erase block of 32 kB of the flash
/// Errors can be checked (and cleared) using w25x_fetch_error()
void w25x_block32_erase(uint32_t addr);

/// Erase block of 64 kB of the flash
/// Errors can be checked (and cleared) using w25x_fetch_error()
void w25x_block64_erase(uint32_t addr);

/// Erase the whole flash memory
///
/// This operation can not be suspended, so it can not be used
/// during print.
/// Errors can be checked (and cleared) using w25x_fetch_error()
void w25x_chip_erase(void (*wait_callback)() = NULL);

/// Fetch and clear error of a previous operation.
/// Returns 0 if there hasn't been any error
int w25x_fetch_error();

/// This should be called when the underlying SPI's DMA finishes DMA transfer (send)
void w25x_spi_transfer_complete_callback();

/// This should be called when the underlying SPI's DMA finishes DMA transfer (receive)
void w25x_spi_receive_complete_callback();

/// This should be called when the underlying SPI's DMA encounters error
void w25x_spi_error_callback();
