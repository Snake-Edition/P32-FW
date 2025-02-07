/// @file
#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

/// Generic interface for reading bytes into buffer.
class AbstractByteReader {
public:
    /// Read bytes into provided buffer, return valid subspan of that buffer.
    virtual std::span<std::byte> read(std::span<std::byte> buffer) = 0;

    /// Wrapper for clients who haven't embraced beauty of std::byte yet.
    std::span<uint8_t> read(std::span<uint8_t> buffer) {
        std::span<std::byte> in { (std::byte *)buffer.data(), buffer.size() };
        std::span<std::byte> out { read(in) };
        return { (uint8_t *)out.data(), out.size() };
    }
};
