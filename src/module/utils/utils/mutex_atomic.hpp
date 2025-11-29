/// @file
#pragma once

#include <atomic>
#include <mutex>

#include <utils/uncopyable.hpp>

/// Class that provides atomic operations over larger or non-primitive types, at the cost of the added mutex
template <typename Value_, typename Mutex_>
class MutexAtomic : public Uncopyable {
#ifndef UNITTESTS
    static_assert(!std::atomic<Value_>::is_always_lock_free, "Use std::atomic instead");
#endif

public:
    using Value = Value_;
    using Mutex = Mutex_;

public:
    MutexAtomic() = default;
    MutexAtomic(const Value &init_val)
        : value_(init_val) {}

    [[nodiscard]] Value load() const {
        std::lock_guard lock { mutex_ };
        return value_;
    }

    void store(const Value &new_value) {
        std::lock_guard lock { mutex_ };
        value_ = new_value;
    }

    inline MutexAtomic &operator=(const Value &new_value) {
        store(new_value);
        return *this;
    }

    /// Calls f(value) while the mutex is locked
    /// @returns result of f(value)
    template <typename F>
    inline auto transform(F &&f) {
        std::lock_guard lock { mutex_ };
        return f(value_);
    }

    /// Calls f(value) while the mutex is locked
    /// @returns result of f(value)
    template <typename F>
    inline auto visit(F &&f) const {
        std::lock_guard lock { mutex_ };
        return f(value_);
    }

private:
    Value value_;
    Mutex mutex_;
};
