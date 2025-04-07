#include <freertos/binary_semaphore.hpp>

#include <cstdlib>

// FreeRTOS.h must be included before semphr.h
#include <FreeRTOS.h>
#include <semphr.h>

namespace freertos {

BinarySemaphore::BinarySemaphore() {
    // If these asserts start failing, go fix the constants.
    static_assert(semaphore_storage_size == sizeof(StaticSemaphore_t));
    static_assert(semaphore_storage_align == alignof(StaticSemaphore_t));

    handle = xSemaphoreCreateBinaryStatic(reinterpret_cast<StaticSemaphore_t *>(&semaphore_storage));
}

BinarySemaphore::~BinarySemaphore() {
    vSemaphoreDelete(SemaphoreHandle_t(handle));
}

void BinarySemaphore::release() {
    if (xSemaphoreGive(SemaphoreHandle_t(handle)) != pdTRUE) {
        // Since the semaphore was obtained correctly, this should never happen.
        std::abort();
    }
}

void BinarySemaphore::release_from_isr() {
    BaseType_t higher_priority_task_woken = pdFALSE;
    if (xSemaphoreGiveFromISR(SemaphoreHandle_t(handle), &higher_priority_task_woken) != pdTRUE) {
        // Since the semaphore was obtained correctly, this should never happen.
        std::abort();
    }
    portYIELD_FROM_ISR(higher_priority_task_woken);
}

void BinarySemaphore::release_blocking() {
    // Same as xSemaphoreGive macro, just the ticksToWait is portMAX_DELAY
    if (xQueueGenericSend(SemaphoreHandle_t(handle), NULL, portMAX_DELAY, queueSEND_TO_BACK) != pdTRUE) {
        static_assert(INCLUDE_vTaskSuspend);
        // Since we are waiting forever and have task suspension, this should never happen.
        std::abort();
    }
}

void BinarySemaphore::acquire() {
    if (xSemaphoreTake(SemaphoreHandle_t(handle), portMAX_DELAY) != pdTRUE) {
        static_assert(INCLUDE_vTaskSuspend);
        // Since we are waiting forever and have task suspension, this should never happen.
        std::abort();
    }
}

bool BinarySemaphore::try_acquire_for(uint32_t ms) {
    return xSemaphoreTake(SemaphoreHandle_t(handle), ms * portTICK_PERIOD_MS) == pdTRUE;
}

} // namespace freertos
