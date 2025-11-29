#include "eeprom_storage.hpp"

#include <common/st25dv64k.h>

uint8_t EEPROMStorage::read_byte(uint16_t address) {
    return st25dv64k_user_read(address);
};
void EEPROMStorage::read_bytes(uint16_t address, std::span<uint8_t> buffer) {
    st25dv64k_user_read_bytes(address, buffer.data(), buffer.size());
};
void EEPROMStorage::write_byte(uint16_t address, uint8_t data) {
    st25dv64k_user_write(address, data);
    bytes_written_.fetch_add(1, std::memory_order_relaxed);
};
void EEPROMStorage::write_bytes(uint16_t address, std::span<const uint8_t> data) {
    st25dv64k_user_write_bytes(address, data.data(), data.size());
    bytes_written_.fetch_add(data.size(), std::memory_order_relaxed);
};

void EEPROMStorage::erase_area(uint16_t start_address, uint16_t end_address) {
    static constexpr std::array<uint8_t, 4> data { 0xff, 0xff, 0xff, 0xff };
    for (; start_address < end_address; start_address += data.size()) {
        write_bytes(start_address, data);
    }
}
