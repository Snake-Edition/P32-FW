#pragma once

#include "power_panic.hpp"
#include <cstdlib>
#include <device/hal.h>

// Abort if reinitialization of the SPI bus fails.
//
// However, in case we are in power panic, just silently continue. In that case:
// * The chance of something failing is rather high (and it's not really a bug,
//   we are in a brownout, the bus might already be dead).
// * If we abort now, we lose the print state _for sure_. If we don't abort, we
//   have some non-zero chance the print state gets stored anyway and save the
//   print - the SPI is used, for example, to turn off the LEDs - and we turn
//   it off because we want to raise our chances of having enough power to save
//   the state; giving up after failing to save some power is contraproductive.
#define CHECK_PP_OR_ABORT(CALL)               \
    if ((CALL) != HAL_OK) {                   \
        if (power_panic::panic_is_active()) { \
            enabled = false;                  \
            return;                           \
        } else {                              \
            abort();                          \
        }                                     \
    }

class [[nodiscard]] SPIBaudRatePrescalerGuard {
private:
    SPI_HandleTypeDef *hspi;
    uint32_t old_prescaler;
    bool enabled;

public:
    SPIBaudRatePrescalerGuard(SPI_HandleTypeDef *hspi, uint32_t new_prescaler, bool enable = true)
        : hspi { hspi }
        , old_prescaler { hspi->Init.BaudRatePrescaler }
        , enabled { enable && old_prescaler < new_prescaler } {
        if (!enabled) {
            return;
        }

        CHECK_PP_OR_ABORT(HAL_SPI_DeInit(hspi));

        hspi->Init.BaudRatePrescaler = new_prescaler;
        CHECK_PP_OR_ABORT(HAL_SPI_Init(hspi));
    }

    ~SPIBaudRatePrescalerGuard() {
        if (!enabled) {
            return;
        }

        CHECK_PP_OR_ABORT(HAL_SPI_DeInit(hspi));
        hspi->Init.BaudRatePrescaler = old_prescaler;
        CHECK_PP_OR_ABORT(HAL_SPI_Init(hspi));
    }
};
