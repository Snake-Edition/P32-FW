#pragma once
#include <span>
#include <stdint.h>
#include <atomic>

namespace configuration_store {

class Storage {

public:
    virtual uint8_t read_byte(uint16_t address) = 0;
    virtual void read_bytes(uint16_t address, std::span<uint8_t> buffer) = 0;
    virtual void write_byte(uint16_t address, uint8_t data) = 0;
    virtual void write_bytes(uint16_t address, std::span<const uint8_t> data) = 0;
    virtual void erase_area(uint16_t start_address, uint16_t end_address) = 0;
    Storage() = default;
    Storage(const Storage &other) = delete;
    Storage(Storage &&other) = delete;
    Storage &operator=(const Storage &other) = delete;
    Storage &operator=(Storage &&other) = delete;

public:
    auto bytes_written() const {
        return bytes_written_.load(std::memory_order_relaxed);
    }

protected:
    /// Number of bytes written to the EEPROM - metric
    std::atomic<uint32_t> bytes_written_ = 0;
};
} // namespace configuration_store
