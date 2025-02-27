#include <common/w25x_communication.hpp>

#include <freertos/binary_semaphore.hpp>
#include <common/w25x.hpp>
#include "string.h"
#include <logging/log.hpp>
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include <buddy/ccm_thread.hpp>
#include "stm32f4xx_hal.h"

LOG_COMPONENT_REF(W25X);

/// Timeout for SPI operations
static const uint32_t TIMEOUT_MS = 1000;

/// Buffer located in SRAM (not in core-coupled RAM)
/// It is used when we user gives us a buffer located in core-coupled RAM, which we can't pass to the DMA.
static uint8_t block_buffer[128];

/// Pending error from last operation(s)
static int current_error = 0;

/// Check whether there is no pending error
static inline bool no_error();

/// Set current error
static inline void set_error(int error);

/// Check whether buffer at given location can be passed to a DMA
/// (the buffer must be in standard ram, not the core-coupled one)
static inline bool memory_supports_dma_transfer(const void *location);

/// Check whether we can use the DMA from the current context
static inline bool dma_is_available();

/// Note that we use the same semaphore for both receive and transmit,
/// there must only be one operation in flight anyway.
static freertos::BinarySemaphore dma_semaphore;

/// Status returned from DMA callbacks.
static volatile HAL_StatusTypeDef dma_status;

/// Setup DMA status and release semaphore
static void release_dma_from_isr(HAL_StatusTypeDef status) {
    dma_status = status;
    dma_semaphore.release_from_isr();
}

/// Receive data over DMA
static HAL_StatusTypeDef receive_dma(uint8_t *buffer, uint32_t len) {
    assert(can_be_used_by_dma(buffer));
    const HAL_StatusTypeDef status = HAL_SPI_Receive_DMA(&SPI_HANDLE_FOR(flash), buffer, len);
    if (status == HAL_OK) {
        dma_semaphore.acquire();
        return dma_status;
    } else {
        return status;
    }
}

/// Send data over DMA
static HAL_StatusTypeDef send_dma(const uint8_t *buffer, uint32_t len) {
    assert(can_be_used_by_dma(buffer));
    const HAL_StatusTypeDef status = HAL_SPI_Transmit_DMA(&SPI_HANDLE_FOR(flash), (uint8_t *)buffer, len);
    if (status == HAL_OK) {
        dma_semaphore.acquire();
        return dma_status;
    } else {
        return status;
    }
}

int w25x_fetch_error() {
    int error = current_error;
    current_error = 0;
    return error;
}

void w25x_receive(uint8_t *buffer, uint32_t len) {
    if (!no_error()) {
        return;
    }

    if (len > 1 && dma_is_available()) {
        if (memory_supports_dma_transfer(buffer)) {
            set_error(receive_dma(buffer, len));
            return;
        } else {
            while (no_error() && len) {
                uint32_t block_len = len > sizeof(block_buffer) ? sizeof(block_buffer) : len;
                set_error(receive_dma(block_buffer, block_len));
                memcpy(buffer, block_buffer, block_len);
                buffer += block_len;
                len -= block_len;
            }
        }
    } else {
        HAL_StatusTypeDef status = HAL_SPI_Receive(&SPI_HANDLE_FOR(flash), buffer, len, TIMEOUT_MS);
        set_error(status);
    }
}

uint8_t w25x_receive_byte() {
    uint8_t byte;
    w25x_receive(&byte, 1);
    return byte;
}

void w25x_send(const uint8_t *buffer, uint32_t len) {
    if (len > 1 && dma_is_available()) {
        if (memory_supports_dma_transfer(buffer)) {
            set_error(send_dma(buffer, len));
        } else {
            while (no_error() && len) {
                uint32_t block_len = len > sizeof(block_buffer) ? sizeof(block_buffer) : len;
                memcpy(block_buffer, buffer, block_len);
                set_error(send_dma(block_buffer, block_len));
                buffer += block_len;
                len -= block_len;
            }
        }
    } else {
        HAL_StatusTypeDef status = HAL_SPI_Transmit(&SPI_HANDLE_FOR(flash), (uint8_t *)buffer, len, TIMEOUT_MS);
        set_error(status);
    }
}

void w25x_send_byte(uint8_t byte) {
    w25x_send(&byte, sizeof(byte));
}

static inline void set_error(int error) {
    current_error = error;
}

static inline bool no_error() {
    return current_error == 0;
}

static inline bool memory_supports_dma_transfer(const void *location) {
    return (uintptr_t)location >= 0x20000000;
}

static inline bool dma_is_available() {
    return !xPortIsInsideInterrupt() && xTaskGetSchedulerState() == taskSCHEDULER_RUNNING;
}

void w25x_spi_transfer_complete_callback() {
    release_dma_from_isr(HAL_OK);
}

void w25x_spi_receive_complete_callback() {
    release_dma_from_isr(HAL_OK);
}

void w25x_spi_error_callback() {
    release_dma_from_isr(HAL_ERROR);
}

void w25x_set_error(int error) {
    set_error(error);
}
