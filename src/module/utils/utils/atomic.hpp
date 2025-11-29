/// \file
#pragma once

#include <atomic>

template <typename T, std::memory_order ORDER = std::memory_order::seq_cst>
class Atomic {
public:
    explicit Atomic() = default;

    explicit Atomic(T val) noexcept
        : value(val) {
    }

    Atomic &operator=(T val) noexcept {
        value.store(val, ORDER);
        return *this;
    }

    void store(const T val) noexcept {
        value.store(val, ORDER);
    }

    T load() const {
        return value.load(ORDER);
    }

private:
    std::atomic<T> value {};
};

template <typename T>
using RelaxedAtomic = Atomic<T, std::memory_order_relaxed>;
