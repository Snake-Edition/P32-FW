#pragma once

#include <charconv>
#include <string>
#include <string.h>
#include <array>
#include <cstdint>
#include <algorithm>
#include <assert.h>
#include "../lang/string_view_utf8.hpp"
#include "../guiapi/include/font_flags.hpp"
#include <bitset>

/// Result of from_chars_light
struct from_chars_light_result {
    const char *ptr; // pointer to last character processed
    std::errc ec; // error code
};

/**
 * @brief from_chars_light functions are lightweight alternatives to std::from_char functions.
 * @note Problem with std::from_chars in GCC is that it uses fastfloat library, which takes about 20KB of flash space.
         from_chars_light functions aims to provide safe, but lightweight alternative. Note that this implementation surely has caveats and different behaviour
         than std implementation, but its is better than any other std functions, because it checks bounds of input array and also some overflows.
   @warning some overflows might not be detected, specifically for 64bit types, its limitation of strtoll function.
 */
template <typename T>
    requires std::integral<T>
from_chars_light_result from_chars_light(const char *first, const char *last, T &value, int base = 10); // from_chars for integers
from_chars_light_result from_chars_light(const char *first, const char *last, float &value); // from_chars for flaoat

/// A const char* that is guaranteed to have unlimited lifetime (thanks to the consteval constructor)
struct ConstexprString {
    constexpr ConstexprString() = default;
    constexpr ConstexprString(const ConstexprString &) = default;
    consteval ConstexprString(const char *str)
        : str_(str) {
        // Make sure the pointer contents is also accessible at compile time
        [[maybe_unused]] const auto ch = str ? str[0] : '\0';
    }

    /// Constructor where the contents of the pointer does not have to be constexpr
    /// !!! Unsafe, use only when you know what you're doing
    static consteval ConstexprString from_str_unsafe(const char *str) {
        ConstexprString r;
        r.str_ = str;
        return r;
    }

    constexpr operator const char *() const {
        return str_;
    }

private:
    const char *str_ = nullptr;
};

/// String that can be passed as a template parameter (use "XX"_tstr)
template <char... chars>
struct TemplateString {
    static constexpr inline const char str[] = { chars..., '\0' };

    consteval inline operator const char *() const {
        return str;
    }
    consteval inline operator ConstexprString() const {
        return ConstexprString(str);
    }
};

template <typename T, T... chars>
constexpr TemplateString<chars...> operator""_tstr() { return {}; }

////////////////////////////////////////////////////////////////////////////////
///
/// numbers of characters in lines
class RectTextLayout {
public:
    RectTextLayout(StringReaderUtf8 &reader, uint16_t max_cols, uint16_t max_rows, is_multiline multiline);

    uint8_t get_width_in_chars() const { return longest_char_cnt; }

    uint8_t get_height_in_chars() const { return get_line_count(); }

    bool has_text_overflown() const { return overflow; }

    uint8_t get_line_characters(uint8_t line) const { return data[line]; }

    uint8_t get_line_count() const;

    bool get_skip_char_on_line(uint8_t line) const { return skip_char[line]; }

private:
    static constexpr size_t MaxLines = 31;
    static constexpr uint8_t MaxCharInLine = 255; // uint8_t
    using Data_t = std::array<uint8_t, MaxLines>;
    std::bitset<MaxLines> skip_char = {};

    Data_t data = {};
    uint8_t current_line = 0;
    uint8_t longest_char_cnt = 0;
    bool overflow = false;

    void set_current_line_characters(uint8_t char_cnt);

    uint8_t get_current_line_characters() const;

    bool new_line(uint8_t max_rows);
};

/// Class that allows safely building strings in char buffers.
/// The class ensures that the string is always validly null terminated. Last character in the buffer is always reserved for \0.
/// Use this class instead of snprintf, because it does all the checks.
/// The class does NOT take the OWNERSHIP in the buffer in any way, the buffer has to exist for the whole existence of StringBuilder (but see ArrayStringBuilder)
/// The builder always considers terminating \0, so the actual available size for string is buffer_size - 1
class StringBuilder {

public:
    StringBuilder() = default;

    StringBuilder(std::span<char> span) {
        init(span.data(), span.size());
    }

    /// See StringBuilder::init
    static StringBuilder from_ptr(char *buffer, size_t buffer_size) {
        StringBuilder result;
        result.init(buffer, buffer_size);
        return result;
    }

public:
    /// Initializes string builder on the buffer.
    void init(char *buffer, size_t buffer_size);

public:
    /// Returns false if there is no space left to write further text or if any of the called functions failed
    inline bool is_ok() const {
        return is_ok_;
    }

    /// See is_ok
    inline bool is_problem() const {
        return !is_ok_;
    }

    /// Returns number of characters in the buffer (NOT counting the terminating \0 which is always there)
    inline size_t char_count() const {
        return current_pos_ - buffer_start_;
    }

    /// Returns number of bytes used of the buffer (COUNTING the terminating \0 which is always there)
    inline size_t byte_count() const {
        return char_count() + 1;
    }

    /// Returns pointer to the first character in the builder buffer
    inline const char *begin() const {
        return buffer_start_;
    }

    /// Returns pointer behind the last character in the builder buffer (should always point to \0)
    inline const char *end() const {
        return current_pos_;
    }

public:
    StringBuilder &append_char(char ch);

    StringBuilder &append_string(const char *str);

    StringBuilder &append_std_string_view(const std::string_view &view);

    StringBuilder &append_string_view(const string_view_utf8 &str);

    /// Appends text to the builder, using vsnprintf under the hood.
    StringBuilder &__attribute__((format(__printf__, 2, 3)))
    append_printf(const char *fmt, ...);

    /// Appends text to the builder, using vsnprintf under the hood.
    StringBuilder &append_vprintf(const char *fmt, va_list args);

    struct AppendFloatConfig {
        /// Maximum decimal places to print
        uint8_t max_decimal_places = 3;

        /// Always use all max_decimal_places
        bool all_decimal_places : 1 = false;

        /// 0.xxx -> .xxx
        bool skip_zero_before_dot : 1 = false;
    };

    /// Appends a float value
    StringBuilder &append_float(double val, const AppendFloatConfig &config);

public:
    /// Allocates $cnt chars at the end of the string and returns the pointer to them.
    /// Appends \0 at the end of the allocation.
    /// Returns pointer to the allocated string or nullptr on failure
    char *alloc_chars(size_t cnt);

private:
    /// For safety reasons, string builder copying is only for static constructors
    StringBuilder(const StringBuilder &o) = default;

private:
    /// Pointer to start of the buffer
    char *buffer_start_ = nullptr;

    /// Pointer after end of the buffer (1 char after the terminating null character)
    char *buffer_end_ = nullptr;

    /// Pointer to the current writable position on the buffer (should always contain terminating null char)
    char *current_pos_ = nullptr;

    /// Error flag, can be set by for example when printf fails
    bool is_ok_ = true;
};

/// StringBuilder bundled together with an array
template <size_t array_size_>
class ArrayStringBuilder : public StringBuilder {

public:
    inline ArrayStringBuilder()
        : StringBuilder(array) {}

public:
    inline const char *str() const {
        if (is_ok()) {
            return str_nocheck();
        } else {
            abort();
        }
    }
    inline const char *str_nocheck() const {
        return array.data();
    }

    inline const uint8_t *str_bytes() const {
        assert(is_ok());
        return reinterpret_cast<const uint8_t *>(array.data());
    }

private:
    std::array<char, array_size_> array;
};
