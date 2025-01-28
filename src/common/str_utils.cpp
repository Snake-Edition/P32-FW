#include "str_utils.hpp"

#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <cinttypes>
#include <string_view_utf8.hpp>

RectTextLayout::RectTextLayout(StringReaderUtf8 &reader, uint16_t max_cols, uint16_t max_rows, is_multiline multiline) {
    if (max_cols == 0 || max_rows == 0) {
        overflow = (reader.getUtf8Char() != 0);
        return;
    }

    if (multiline == is_multiline::no) {
        max_rows = 1;
    }

    std::optional<int> last_space_index = std::nullopt;
    unichar c = 0;
    int chars_this_line = 0;

    while ((c = reader.getUtf8Char()) != 0) {
        switch (c) {
        case '\n': // new line

            set_current_line_characters(chars_this_line);
            if (!new_line(max_rows)) {
                return;
            }

            chars_this_line = 0;
            last_space_index = std::nullopt;
            break;

        case ' ': // remember space position

            // erasing start space is not handled here
            // Whenever we enter new line (except the first one), we always skip 1 char ('\n' || ' ') from the stream
            // If there are more whitespace characters, its clearly a choice

            last_space_index = chars_this_line;
            [[fallthrough]];

        default:
            chars_this_line++;

            if (chars_this_line > max_cols) {

                if (!last_space_index || current_line == max_rows - 1) { // Do not wrap singleline texts
                    last_space_index = max_cols;
                    overflow = true;
                }

                // It does not count newline char and space before wrapped word
                // Wrapping cuts overflown word and put it on the next line
                // If word is too long for a line, it will return calculated layout up until that error

                // count chars in next line
                chars_this_line -= ((*last_space_index) + 1); // +1 space
                set_current_line_characters(*last_space_index);
                last_space_index = std::nullopt;

                if (!new_line(max_rows)) {
                    return;
                }
            }
        }
    }

    set_current_line_characters(chars_this_line);
    return;
}

void RectTextLayout::set_current_line_characters(uint8_t char_cnt) {
    data[current_line] = char_cnt;
    longest_char_cnt = std::max(longest_char_cnt, char_cnt);
}

uint8_t RectTextLayout::get_current_line_characters() const {
    return get_line_characters(current_line);
}

uint8_t RectTextLayout::get_line_count() const {
    return current_line == 0 && get_current_line_characters() == 0 ? 0 : current_line + 1;
}

bool RectTextLayout::new_line(uint8_t max_rows) {
    if (current_line >= MaxLines || current_line + 1 >= max_rows) {
        overflow = true;
        return false;
    }
    current_line++;
    return true;
}

// StringBuilder
// ---------------------------------------------
void StringBuilder::init(char *buffer, size_t buffer_size) {
    buffer_start_ = buffer;
    current_pos_ = buffer;
    buffer_end_ = buffer + buffer_size;

    // Make the resulting string valid from the go
    *current_pos_ = '\0';
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
    if (isnan(val)) {
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

template <typename T, auto strtox_func>
from_chars_light_result from_chars_light_common(const char *first, const char *last, T &value, int base) {
    std::array<char, sizeof(value) * 8 + 2> buffer; // buffer where we'll copy the number with ending zero, size reserved is to fit base 2 number + sign, plus ending zero.

    size_t len = std::distance(first, last);
    if (len > buffer.size() - 1) {
        // buffer is too small, or won't fit ending zero
        return { first, std::errc::value_too_large };
    }
    std::copy(first, last, buffer.begin());
    buffer[len] = '\0'; // make sure there is ending zero, to avoid strtoi reading beyond the buffer
    char *out_end = nullptr;
    errno = 0;
    auto value_res = strtox_func(buffer.data(), &out_end, base);
    if (errno != 0) {
        return { out_end, std::errc::result_out_of_range };
    }
    if (out_end == buffer.data()) {
        return { out_end, std::errc::invalid_argument };
    }
    if (value_res < std::numeric_limits<T>::min() || value_res > std::numeric_limits<T>::max()) {
        return { out_end, std::errc::result_out_of_range };
    }
    value = value_res;

    return { out_end, std::errc {} };
}

template <>
from_chars_light_result from_chars_light(const char *first, const char *last, unsigned long long &value, int base) {
    static_assert(sizeof(unsigned long long) >= sizeof(value));
    return from_chars_light_common<unsigned long long, std::strtoull>(first, last, value, base);
}

template <>
from_chars_light_result from_chars_light(const char *first, const char *last, unsigned long &value, int base) {
    static_assert(sizeof(unsigned long) >= sizeof(value));
    return from_chars_light_common<unsigned long, std::strtoul>(first, last, value, base);
}

template <>
from_chars_light_result from_chars_light(const char *first, const char *last, unsigned int &value, int base) {
    static_assert(sizeof(unsigned long) >= sizeof(value));
    return from_chars_light_common<unsigned int, std::strtoul>(first, last, value, base);
}

template <>
from_chars_light_result from_chars_light(const char *first, const char *last, unsigned short &value, int base) {
    static_assert(sizeof(unsigned long) >= sizeof(value));
    return from_chars_light_common<unsigned short, std::strtoul>(first, last, value, base);
}

template <>
from_chars_light_result from_chars_light(const char *first, const char *last, unsigned char &value, int base) {
    static_assert(sizeof(unsigned long) >= sizeof(value));
    return from_chars_light_common<unsigned char, std::strtoul>(first, last, value, base);
}

template <>
from_chars_light_result from_chars_light(const char *first, const char *last, long long &value, int base) {
    static_assert(sizeof(long long) >= sizeof(value));
    return from_chars_light_common<long long, std::strtoll>(first, last, value, base);
}

template <>
from_chars_light_result from_chars_light(const char *first, const char *last, long &value, int base) {
    static_assert(sizeof(long) >= sizeof(value));
    return from_chars_light_common<long, std::strtol>(first, last, value, base);
}

template <>
from_chars_light_result from_chars_light(const char *first, const char *last, int &value, int base) {
    static_assert(sizeof(long) >= sizeof(value));
    return from_chars_light_common<int, std::strtol>(first, last, value, base);
}

template <>
from_chars_light_result from_chars_light(const char *first, const char *last, short &value, int base) {
    static_assert(sizeof(long) >= sizeof(value));
    return from_chars_light_common<short, std::strtol>(first, last, value, base);
}

template <>
from_chars_light_result from_chars_light(const char *first, const char *last, signed char &value, int base) {
    static_assert(sizeof(long) >= sizeof(value));
    return from_chars_light_common<signed char, std::strtol>(first, last, value, base);
}

from_chars_light_result from_chars_light(const char *first, const char *last, float &value) {
    std::array<char, 32> buffer; // buffer where we'll copy the number with ending zero

    size_t len = std::distance(first, last);
    if (len >= buffer.size() - 1) {
        return { first, std::errc::value_too_large };
    }
    std::copy(first, last, buffer.begin());
    buffer[len] = '\0'; // make sure there is ending zero, to avoid strtoi reading beyond the buffer
    char *out_end = nullptr;
    value = std::strtof(buffer.data(), &out_end);
    if (out_end == buffer.data()) {
        return { first, std::errc::invalid_argument };
    }
    return { out_end, std::errc {} };
}
