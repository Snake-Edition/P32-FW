#pragma once
#include <array>

/**
 * @brief Used to discern whether template parameter is std::array<..., ...> or not
 */
template <typename T>
struct is_std_array : std::false_type {};
template <typename T, std::size_t N>
struct is_std_array<std::array<T, N>> : std::true_type {};

template <typename T>
inline constexpr bool is_std_array_v = is_std_array<T>::value;

namespace stdext {

/// Maps each item \p x in \p array to \p f(x) in the result array
/// \returns array with of the mapped values
template <typename T, size_t size, typename F>
constexpr auto map_array(const std::array<T, size> &array, F &&f) {
    std::array<std::remove_cvref_t<decltype(f(array[0]))>, size> result;
    for (size_t i = 0; i < size; i++) {
        result[i] = f(array[i]);
    }
    return result;
}

/// Returns a sub-array of the input container of size \p new_size
template <size_t new_size>
constexpr auto array_sub_copy(auto &&source, size_t offset = 0) {
    using Source = std::remove_cvref_t<decltype(source)>;
    static_assert(new_size <= std::tuple_size_v<Source>);
    assert(offset + new_size <= source.size());

    std::array<typename Source::value_type, new_size> result {};
    std::copy(source.begin() + offset, source.begin() + offset + new_size, result.begin());
    return result;
}

/// Concatenates two std::array objects of the same value_type
template <typename T, size_t N1, size_t N2>
constexpr auto array_concat(const std::array<T, N1> &a1, const std::array<T, N2> &a2) {
    std::array<T, N1 + N2> result {};
    std::copy(a1.begin(), a1.end(), result.begin());
    std::copy(a2.begin(), a2.end(), result.begin() + N1);
    return result;
}

} // namespace stdext
