/// @file
#pragma once

#include <bitset>
#include <utility>

/// A wrapper over a std::bitset with some small extra features to work better for enum indexing:
/// - Initializer takes std::pair<Enum, bool> and checks that the Enum matches the array index (at compile time)
/// - Constructor is consteval and the initializer must be the same size as \p cnt
/// - Bitset now indexable with the enum type, no need to cast to_underlying
///
/// Functionally, this is equivalent to std::bitset<cnt>.
template <typename Enum, auto cnt>
struct EnumBitset final : public std::bitset<static_cast<size_t>(cnt)> {
    using Bitset = std::bitset<static_cast<size_t>(cnt)>;

    constexpr EnumBitset() noexcept {}

    explicit consteval EnumBitset(std::initializer_list<std::pair<Enum, bool>> items) noexcept {
        // Check that the sizes match
        if (items.size() != static_cast<size_t>(cnt)) {
            std::terminate();
        }

        size_t i = 0;
        for (const auto &pair : items) {
            this->set(pair.first, pair.second);

            // Check that indexes match
            if (static_cast<size_t>(pair.first) != i) {
                std::terminate();
            }

            i++;
        }
    }

    using Bitset::test;
    [[nodiscard]] inline constexpr bool test(Enum pos) const {
        return Bitset::test(std::to_underlying(pos));
    }

    using Bitset::set;
    inline constexpr EnumBitset &set(Enum pos, bool value = true) {
        Bitset::set(std::to_underlying(pos), value);
        return *this;
    }

    using Bitset::reset;
    inline constexpr EnumBitset &reset(Enum pos) {
        Bitset::reset(std::to_underlying(pos));
        return *this;
    }

    using Bitset::flip;
    inline constexpr EnumBitset &flip(Enum pos) {
        Bitset::flip(std::to_underlying(pos));
        return *this;
    }

    using Bitset::operator[];
    [[nodiscard]] inline constexpr bool operator[](Enum pos) const {
        return Bitset::operator[](std::to_underlying(pos));
    }
    [[nodiscard]] inline constexpr Bitset::reference operator[](Enum pos) {
        return Bitset::operator[](std::to_underlying(pos));
    }
};
