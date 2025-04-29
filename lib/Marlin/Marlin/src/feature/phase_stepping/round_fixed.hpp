#pragma once

namespace phase_stepping {

// Round a fixed point number to the nearest integer assuming given number of
// fractional bits.
template <typename T>
T round_fixed(T value, int frac_bits) {
    if (frac_bits == 0) {
        return value;
    }
    // There is a catch: bit shifting for signed values as a mean to divide is
    // implementation specific. On Cortex ARM it rounds down to negative
    // infinity which is exactly what we **DON'T** want.
    //
    // You might wonder - if there such a fuss, why not just use integral
    // division? By doing bit shifts, we still save time as we cannot avoid the
    // branching as we need to either add or subtract the rounding value. With
    // the bit shifts we are at 4 cycles, with division we are at 4-16 cycles.
    if (value < 0) {
        value = -value + (1 << (frac_bits - 1));
        value = value >> frac_bits;
        return -value;
    } else {
        value = value + (1 << (frac_bits - 1));
        return value >> frac_bits;
    }
}

} // namespace phase_stepping
