#pragma once

#include <type_traits>
#include <array>
#include <cstdint>
#include <stddef.h>

namespace freertos {

/// C++ wrapper for FreeRTOS binary semaphore.
///
/// Gets created with 0 permits in it (if acquire is called first, it'll
/// block).
class BinarySemaphore {
public:
    // We use erased storage in order to not pollute the scope with FreeRTOS internals.
    // The actual size and alignment are statically asserted in implementation file.
#ifdef UNITTESTS
    static constexpr size_t semaphore_storage_size = 168;
    static constexpr size_t semaphore_storage_align = 8;
#else
    static constexpr size_t semaphore_storage_size = 80;
    static constexpr size_t semaphore_storage_align = 4;
#endif
    using Storage = std::array<uint8_t, semaphore_storage_size>;

private:
    alignas(semaphore_storage_align) Storage semaphore_storage;

public:
    BinarySemaphore();
    ~BinarySemaphore();
    BinarySemaphore(const BinarySemaphore &) = delete;
    BinarySemaphore &operator=(const BinarySemaphore &) = delete;

    /// Increments the internal counter and unblocks one acquirer
    ///
    /// Raises a BSOD if the semaphore already contains one permit (eg. if you
    /// create the semaphore and call release twice without acquire in
    /// between).
    void release();

    /// Just like release, but can be called from an interrupt.
    ///
    /// (Note: We cheat about the typedef here a bit to avoid include hell).
    [[nodiscard]] long release_from_isr();

    /// Same as \p release, but instead of raising a bsod when there's already
    /// a permit, it blocks for an acquire to consume it first before
    /// returning (and setting another permit there).
    void release_blocking();

    /// Consumes one permit from the semaphore, blocking until it's available
    /// as necessary.
    void acquire();
};

} // namespace freertos