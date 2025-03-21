#pragma once

#include <type_traits>
#include <concepts>

namespace detail {

template <typename T, typename... Args>
consteval bool is_brace_constructible() {
    return requires { T { std::declval<Args>()... }; };
};

template <typename Derived = void>
struct UniversalType {
    template <typename T>
    operator T &&() const
        requires(!std::is_base_of_v<T, Derived> && std::is_move_constructible_v<T>)
    {
        return *reinterpret_cast<T *>(0);
    }

    template <typename T>
    operator T &() const
        requires(!std::is_base_of_v<T, Derived> && std::is_copy_constructible_v<T>)
    {
        return *reinterpret_cast<T *>(0);
    }

    template <typename T>
    operator T() const
        requires(!std::is_base_of_v<T, Derived> && !std::is_copy_constructible_v<T> && !std::is_move_constructible_v<T>)
    {
    }

    static constexpr bool is_no_base = !std::is_same_v<Derived, void>;
};

/**
 * @brief Counts the number of members via trying to build the aggregate with increasingly more arguments until it can't be built anymore and stopping there
 *
 * @tparam T Struct to be counted
 */
template <typename T, typename... Member>
consteval size_t number_of_members() {
    // First try converting to a type that is not convertible to a base class of T - exclude parent initializers
    // If that fails, allow converting to a base class, but do not count it to the result

    // We cannot allow both at the same time, that would error at ambiguous constructor

    if constexpr (is_brace_constructible<T, Member..., UniversalType<T>>()) {
        return number_of_members<T, Member..., UniversalType<T>>();

    } else if constexpr (is_brace_constructible<T, Member..., UniversalType<void>>()) {
        return number_of_members<T, Member..., UniversalType<void>>();

    } else {
        return (0 + ... + Member::is_no_base);
    }
}
} // namespace detail

/**
 * @brief Returns the number of members in an aggregate (struct, without any base classes)
 *
 * @tparam T
 */
template <typename T>
consteval auto aggregate_arity() {
    return detail::number_of_members<T>();
}
