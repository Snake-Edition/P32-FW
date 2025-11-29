/**
 * @file utility_extensions.hpp
 * @brief This file contains various extensions to the std <utility> header
 */

#pragma once

#include <utility>

/**
 * @brief If used in consteval functions, will cause a (rather cryptic) compile time error (call to non-constexpr function).
 *  Alternative to static_assert with dependant false
 * @param reason
 */
inline void consteval_assert_false([[maybe_unused]] const char *reason = "") {}

/**
 * @brief Will cause a (rather cryptic) compile time error if condition == false (call to non-constexpr function).
 * @param reason
 */
consteval inline void consteval_assert(bool condition, const char *reason = "") {
    if (!condition) {
        consteval_assert_false(reason);
    }
}

template <typename T, typename... U>
concept is_any_of = (std::same_as<T, U> || ...);
