#pragma once

#include <stdint.h>
#include <cmsis_gcc.h>

namespace buddy {

/**
 * @brief Disable all maskable interrupts
 *
 * Disabling all interrupts may be needed in case of dealing with stepper data
 * or in case timing with nanosecond precision is needed.
 *
 * In other cases use CriticalSection instead.
 *
 * Interrupts are disabled when object is constructed by its default constructor
 * and resumed to original state once destroyed.
 */
class DisableInterrupts {
    uint32_t m_primask;

public:
    DisableInterrupts() {
        m_primask = __get_PRIMASK();
        __disable_irq();
    }

    ~DisableInterrupts() {
        __set_PRIMASK(m_primask);
    }

    DisableInterrupts(const DisableInterrupts &other) = delete;
    DisableInterrupts &operator=(const DisableInterrupts &other) = delete;
};

} // namespace buddy
