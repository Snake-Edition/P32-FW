#pragma once

#include <random.h>
class Custom_uint31_t {
public:
    // Constructor with optional initialization
    Custom_uint31_t(uint32_t value = 0) {
        set_value(value);
    }

    // Get the underlying 31-bit value
    constexpr uint32_t get_value() const {
        return value_ & mask_;
    }

    // Set the underlying 31-bit value
    constexpr void set_value(uint32_t value) {
        value_ = value & mask_;
    }

    Custom_uint31_t operator++(int) {
        Custom_uint31_t temp = *this;
        value_++;
        return temp;
    }

    // Overload comparison operators
    bool operator==(const Custom_uint31_t &other) const {
        return get_value() == other.get_value();
    }

    bool operator!=(const Custom_uint31_t &other) const {
        return !(*this == other);
    }

    // type casting
    uint32_t to_uint32_t() const {
        return static_cast<uint32_t>(get_value());
    }

    // Overload subtraction arithmetic operators, to cover properlyb 31bit overflow
    Custom_uint31_t operator-(const Custom_uint31_t &other) const {
        return Custom_uint31_t(get_value() - other.get_value());
    }

    static inline Custom_uint31_t generate_random_uint31() {
        return static_cast<Custom_uint31_t>(rand_u() & mask_);
    }

private:
    uint32_t value_;
    static const uint32_t mask_ = 0x7FFFFFFF; // Mask for 31 bits
    static const uint32_t max_ = 0x7FFFFFFF; // Maximum value for 31 bits
    static const uint32_t min_ = 0; // Minimum value for 31 bits
};

namespace fsm {
// defined in this file in order to avoid #include dependencies in fsm_states.hpp file
// which breaks compilation of unit tests
using StateId = Custom_uint31_t;
} // namespace fsm
