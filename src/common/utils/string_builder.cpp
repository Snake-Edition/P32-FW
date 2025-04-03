#include "string_builder.hpp"

#include <cstdarg>
#include <cmath>
#include <cinttypes>

#include <bsod.h>

void StringBuilder::init(char *buffer, size_t buffer_size) {
    buffer_start_ = buffer;
    current_pos_ = buffer;
    buffer_end_ = buffer + buffer_size;

    // Make the resulting string valid from the go
    *current_pos_ = '\0';
}

const char *StringBuilder::str() const {
    if (is_ok()) {
        return str_nocheck();
    } else {
        bsod("StringBuilder overflow");
    }
}

StringBuilder &StringBuilder::append_char(char ch) {
    if (char *ptr = alloc_chars(1)) {
        *ptr = ch;
    }

    return *this;
}

StringBuilder &StringBuilder::append_string(const char *str) {
    if (is_problem()) {
        return *this;
    }

    // Accomodate for terminating null
    char *buffer_pre_end = buffer_end_ - 1;
    char *buffer_pos = current_pos_;

    while (true) {
        // At the end of the appended string -> success
        if (*str == '\0') {
            current_pos_ = buffer_pos;
            break;
        }

        // Check if we're not at the end of the buffer
        if (buffer_pos >= buffer_pre_end) {
            is_ok_ = false;
            break;
        }

        *buffer_pos++ = *str++;
    }

    // Ensure the string is valid by appending nullterm
    *current_pos_ = '\0';
    return *this;
}

StringBuilder &StringBuilder::append_std_string_view(const std::string_view &view) {
    if (const auto buf = alloc_chars(view.size())) {
        view.copy(buf, view.size());
    }

    return *this;
}

StringBuilder &StringBuilder::append_string_view(const string_view_utf8 &str) {
    StringReaderUtf8 reader(str);

    while (true) {
        if (is_problem()) {
            return *this;
        }

        char b = reader.getbyte();
        if (b == '\0') {
            return *this;
        }

        append_char(b);
    }

    return *this;
}

StringBuilder &StringBuilder::append_printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    append_vprintf(fmt, args);
    va_end(args);
    return *this;
}

StringBuilder &StringBuilder::append_vprintf(const char *fmt, va_list args) {
    if (is_problem()) {
        return *this;
    }

    const int available_bytes = int(buffer_end_ - current_pos_);
    const int ret = vsnprintf(current_pos_, available_bytes, fmt, args);

    // >= because we need to account fo rhte terminating \0
    if (ret < 0 || ret >= available_bytes) {
        *current_pos_ = '\0';
        is_ok_ = false;
        return *this;
    }

    current_pos_ += ret;
    return *this;
}

// This stupid utility function saves 3kB of FLASH...
// ...unless somebody else uses pow() somewhere, then it can be removed again.
static constexpr uint32_t dumb_pow_10(uint8_t exponent) {
    uint32_t result = 1;
    for (uint8_t i = 0; i < exponent; ++i) {
        result *= 10;
    }
    return result;
}
static_assert(dumb_pow_10(0) == 1);
static_assert(dumb_pow_10(1) == 10);
static_assert(dumb_pow_10(2) == 100);

StringBuilder &StringBuilder::append_float(double val, const AppendFloatConfig &config) {
    if (std::isnan(val)) {
        append_string("NaN");
        return *this;
    }

    const bool is_negative = val < 0;
    uint32_t precision_mult = dumb_pow_10(config.max_decimal_places);
    uint64_t accum = static_cast<uint64_t>(round(abs(val) * precision_mult));

    if (accum == 0) {
        append_char('0');
        return *this;
    }

    if (is_negative) {
        append_char('-');
    }

    // Print integral part
    const auto integral_part = accum / precision_mult;
    accum %= precision_mult;

    if (integral_part > 0 || is_negative || !config.skip_zero_before_dot) {
        append_printf("%" PRIu64, integral_part);
    }

    // Print decimal part
    for (int i = 0; accum > 0 || (config.all_decimal_places && i < config.max_decimal_places); i++) {
        if (i == 0) {
            // Decimal point
            append_char('.');
        }

        precision_mult /= 10;
        append_char('0' + (accum / precision_mult % 10));
        accum %= precision_mult;
    }

    return *this;
}

char *StringBuilder::alloc_chars(size_t cnt) {
    if (is_problem()) {
        return nullptr;
    }

    const size_t available_bytes = int(buffer_end_ - current_pos_);

    // >= because we need to account fo rhte terminating \0
    if (cnt >= available_bytes) {
        is_ok_ = false;
        return nullptr;
    }

    current_pos_ += cnt;
    *current_pos_ = '\0';
    return current_pos_ - cnt;
}
