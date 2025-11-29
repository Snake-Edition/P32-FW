#pragma once
#include <stdint.h>
#include <bitset>
#include <array>
/**
 * | controller |  2811 | 2812 |  2811  |  2812  |
 * |------------|-------|------|--------|--------|
 * |   signal   | Timing [ns] || Tolerance [ns] ||
 * |  T1H [ns]  |   600 |  800 |    150 |    150 |
 * |  T1L [ns]  |   650 |  450 |    150 |    150 |
 * |  T0H [ns]  |   250 |  400 |    150 |    150 |
 * |  T0L [ns]  |  1000 |  850 |    150 |    150 |
 *
 *
 * |          | Timing [ns] | Tolerance [ns]  |
 * |controller| 2811 | 2812 |  2811  |  2812  |
 * |----------|------|------|--------|--------|
 * | T1H [ns] |  600 |  800 |    150 |    150 |
 * | T1L [ns] |  650 |  450 |    150 |    150 |
 * | T0H [ns] |  250 |  400 |    150 |    150 |
 * | T0L [ns] | 1000 |  850 |    150 |    150 |
 *
 * |          |   2811     |   2812     |
 * |          | min |  max | min |  max |
 * |----------|-----|------|-----|------|
 * | T1H [ns] | 450 |  750 | 650 |  950 |
 * | T1L [ns] | 500 |  800 | 300 |  600 |
 * | T0H [ns] | 100 |  400 | 250 |  550 |
 * | T0L [ns] | 850 | 1150 | 700 | 1000 |
 *
 * |          |   281X     |
 * |          | min |  max |
 * |----------|-----|------|
 * | T1H [ns] | 650 |  750 |
 * | T1L [ns] | 500 |  600 |
 * | T0H [ns] | 250 |  400 |
 * | T0L [ns] | 850 | 1000 |
 *
 * | SPI freq [Mhz]    | 21        |
 * | spi t [ns]        | 47.62     |
 * | range             | min | max |
 * |-------------------|-----|-----|
 * | T1H [tick]        | 14  | 15  |
 * | T1L [tick]        | 11  | 12  |
 * | T0H [tick]        | 6   | 8   |
 * | T0L [tick]        | 18  | 21  |
 * | sum log 1 [tick]  | 25  | 27  |
 * | sum log 0 [tick]  | 24  | 29  |
 *
 * |  SPI freq [Mhz]  |           10.5                               ||
 * |------------------|-----------------------------------------------|
 * | spi t [ns]       | 95.24                                        ||
 * | range            | Picked from 21 MHZ Range | Current  frequency |
 * | T1H [tick]       | 14                       | 7                  |
 * | T1L [tick]       | 12                       | 6                  |
 * | T0H [tick]       | 6                        | 3                  |
 * | T0L [tick]       | 18                       | 9                  |
 * |------------------|--------------------------|--------------------|
 * | sum log 1 [tick] | 26                       | 13                 |
 * | sum log 0 [tick] | 24                       | 12                 |
 *
 * | SPI freq [Mhz]   |            7                                 ||
 * |------------------|-----------------------------------------------|
 * | spi t [ns]       | 142.86                                       ||
 * | range            | Picked from 21 MHZ Range | Current  frequency |
 * | T1H [tick]       | 15                       | 5                  |
 * | T1L [tick]       | 12                       | 4                  |
 * | T0H [tick]       | 6                        | 2                  |
 * | T0L [tick]       | 18                       | 6                  |
 * |------------------|-----------------------------------------------|
 * | sum log 1 [tick] | 27                       | 9                  |
 * | sum log 0 [tick] | 24                       | 8                  |
 *
 */

namespace neopixel {
inline constexpr uint32_t T1H_21MHz = 14;
inline constexpr uint32_t T1L_21MHz = 11;
inline constexpr uint32_t T0H_21MHz = 6;
inline constexpr uint32_t T0L_21MHz = 18;
inline constexpr uint32_t RESET_21MHz = 51 * 21;

inline constexpr uint32_t T1H_10M5Hz = 7;
inline constexpr uint32_t T1L_10M5Hz = 6;
inline constexpr uint32_t T0H_10M5Hz = 3;
inline constexpr uint32_t T0L_10M5Hz = 9;
inline constexpr uint32_t RESET_10M5Hz = uint32_t(51.f * 10.5f);

// cannot set prescaller to 7 MHz
// and clock source is fixed
inline constexpr uint32_t T1H_7MHz = 5;
inline constexpr uint32_t T1L_7MHz = 4;
inline constexpr uint32_t T0H_7MHz = 2;
inline constexpr uint32_t T0L_7MHz = 6;
inline constexpr uint32_t RESET_7MHz = 51 * 7;

// 2.5MHz 2812 only
inline constexpr uint32_t T1H_2M5Hz = 2; // 800ns
inline constexpr uint32_t T1L_2M5Hz = 1; // 400ns
inline constexpr uint32_t T0H_2M5Hz = 1; // 400ns
inline constexpr uint32_t T0L_2M5Hz = 2; // 800ns
inline constexpr uint32_t RESET_2M5Hz = uint32_t(51.f * 2.5f);

/**
 * @brief base class for LEDs defining color array for given count
 *
 * @tparam COUNT count of LEDs
 */
template <size_t COUNT>
class LedsBase {
public:
    constexpr LedsBase()
        : leds_to_rewrite(COUNT) {
        leds.fill(0);
    }
    using color_array = std::array<uint32_t, COUNT>;

    void set(uint32_t color, size_t index) {
        if (index >= COUNT) {
            return;
        }

        if (leds[index] == color) {
            return;
        }

        leds[index] = color;
        leds_to_rewrite = std::max(index + size_t(1), leds_to_rewrite);
    }

    void set(const uint32_t *colors, size_t count) {
        count = std::min(count, size_t(COUNT - 1));
        for (size_t led = 0; led <= count; ++led) {
            set(colors[led], led);
        }
    }

    void set(const color_array &colors) {
        set(colors.begin(), colors.size());
    }

    void set_all(uint32_t color) {
        for (size_t i = 0; i < COUNT; ++i) {
            set(color, i);
        }
    }

    void force_refresh(size_t cnt) {
        leds_to_rewrite = cnt;
    }

protected:
    color_array leds;
    size_t leds_to_rewrite;
};

/**
 * @brief base class for SPI LEDs handling conversion LEDs to bitset
 *
 * @tparam COUNT count of LEDs
 * @tparam T1H   lenght of high bus status of converted logical "1" bit value
 * @tparam T1L   lenght of low  bus status of converted logical "1" bit value
 * @tparam T0H   lenght of high bus status of converted logical "0" bit value
 * @tparam T0L   lenght of low  bus status of converted logical "0" bit value
 */
template <size_t COUNT, size_t T1H, size_t T1L, size_t T0H, size_t T0L>
class LedsSPIBase : public LedsBase<COUNT> {
public:
    using color_array = std::array<uint32_t, COUNT>;

    constexpr LedsSPIBase() = default;

protected:
    static constexpr uint32_t max_pulse_len = T1H + T1L > T0H + T0L ? T1H + T1L : T0H + T0L; // std::max( T1H + T1L,  T0H + T0L); - not constexpr
    static constexpr uint32_t max_size = max_pulse_len * 24; // 3*8bit color
    static constexpr uint32_t bitfield_size = COUNT * max_size; // theoretical maximum size in bits
    std::bitset<bitfield_size> led_bitset;

    void set_high(size_t &bitfield_pos) {
        for (size_t i = 0; i < T1H; ++i) {
            led_bitset[bitfield_pos++] = true;
        }

        bitfield_pos += T1L; // no need to set false
    }

    void set_low(size_t &bitfield_pos) {
        for (size_t i = 0; i < T0H; ++i) {
            led_bitset[bitfield_pos++] = true;
        }

        bitfield_pos += T0L; // no need to set false
    }

    /**
     * @brief Set the Bitset object from leds array
     *        call exactly once before sending data to LEDs via SPI
     *
     * @return size_t number of bits to be send
     */
    size_t set_bitset();
};

template <size_t COUNT, size_t T1H, size_t T1L, size_t T0H, size_t T0L>
size_t LedsSPIBase<COUNT, T1H, T1L, T0H, T0L>::set_bitset() {
    if (this->leds_to_rewrite == 0) {
        return 0; // nothing to set
    }

    led_bitset.reset(); // clear bit array

    size_t bitfield_position = 0;

    for (size_t i = 0; i < this->leds_to_rewrite; ++i) {
        std::bitset<24> bits_of_color = this->leds[i];
        for (int8_t bit = 23; bit >= 0; --bit) {
            bits_of_color[bit] ? set_high(bitfield_position) : set_low(bitfield_position); // bitfield_position passed by reference
        }
    }

    this->leds_to_rewrite = 0;
    return bitfield_position;
}

/**
 * @brief child of LedsSPIBase handling MSB data conversion
 *
 * @tparam COUNT count of leds
 * @tparam T1H   lenght of high bus status of converted logical "1" bit value
 * @tparam T1L   lenght of low  bus status of converted logical "1" bit value
 * @tparam T0H   lenght of high bus status of converted logical "0" bit value
 * @tparam T0L   lenght of low  bus status of converted logical "0" bit value
 * @tparam RESET_PULSE number of pulses needed to be in low state to write signals on LEDs
 */
template <size_t COUNT, size_t T1H, size_t T1L, size_t T0H, size_t T0L, size_t RESET_PULSE>
class LedsSPIMSB : public LedsSPIBase<COUNT, T1H, T1L, T0H, T0L> {
protected:
    uint8_t send_buff[(LedsSPIBase<COUNT, T1H, T1L, T0H, T0L>::bitfield_size + RESET_PULSE + 7) / 8];

    /**
     * @brief ready data to send
     *        call exactly once before sending data to LEDs via SPI
     *
     * @return size_t number of bytes to be send
     */
    size_t bitfield_to_send_buf() {
        const size_t bit_count = this->set_bitset();
        if (!bit_count) {
            return 0;
        }

        // write reset pulse (clear each byt it overlaps)
        for (size_t i = bit_count / 8; i < ((bit_count + RESET_PULSE + 7) / 8); ++i) {
            send_buff[i] = 0;
        }

        for (size_t bit_index = 0; bit_index < bit_count; ++bit_index) {
            const uint8_t target_bit = 1 << (7 - (bit_index % 8));
            uint8_t &r_target_byte = send_buff[bit_index / 8];
            r_target_byte = this->led_bitset[bit_index] ? r_target_byte | target_bit : r_target_byte & (~target_bit);
        }

        return (bit_count + RESET_PULSE + 7) / 8;
    };
};

/**
 * @brief draw function pointer
 *
 */
using draw_fn_t = void (*)(uint8_t *, uint16_t);

/**
 * @brief fully functional child of LedsSPIMSB able to set physical LEDs via pointer
 *        to function hadling SPI DMA
 * @tparam COUNT count of leds
 * @tparam DRAW_FN pointer to draw function
 * @tparam T1H   lenght of high bus status of converted logical "1" bit value
 * @tparam T1L   lenght of low  bus status of converted logical "1" bit value
 * @tparam T0H   lenght of high bus status of converted logical "0" bit value
 * @tparam T0L   lenght of low  bus status of converted logical "0" bit value
 * @tparam RESET_PULSE number of pulses needed to be in low state to write signals on LEDs
 */
template <size_t COUNT, draw_fn_t DRAW_FN, size_t T1H, size_t T1L, size_t T0H, size_t T0L, size_t RESET_PULSE>
class LedsSPI : public LedsSPIMSB<COUNT, T1H, T1L, T0H, T0L, RESET_PULSE> {
public:
    void update() {
        if (!this->leds_to_rewrite) {
            return;
        }

        size_t bytes = this->bitfield_to_send_buf();
        DRAW_FN(this->send_buff, bytes);
    };
};

template <size_t COUNT, draw_fn_t DRAW_FN>
using LedsSPI21MHz = LedsSPI<COUNT, DRAW_FN, T1H_21MHz, T1L_21MHz, T0H_21MHz, T0L_21MHz, RESET_21MHz>;

template <size_t COUNT, draw_fn_t DRAW_FN>
using LedsSPI10M5Hz = LedsSPI<COUNT, DRAW_FN, T1H_10M5Hz, T1L_10M5Hz, T0H_10M5Hz, T0L_10M5Hz, RESET_10M5Hz>;

template <size_t COUNT, draw_fn_t DRAW_FN>
using LedsSPI2M5Hz = LedsSPI<COUNT, DRAW_FN, T1H_2M5Hz, T1L_2M5Hz, T0H_2M5Hz, T0L_2M5Hz, RESET_2M5Hz>;

}; // namespace neopixel
