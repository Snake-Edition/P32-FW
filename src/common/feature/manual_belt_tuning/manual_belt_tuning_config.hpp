#pragma once

namespace manual_belt_tuning {

// frequency range
constexpr uint16_t freq_min = 50;
constexpr uint16_t freq_max = 130;

// acceleration range
constexpr uint16_t accel_min = 7000;
constexpr uint16_t accel_max = 10500;

// 1m length belt waight
constexpr float nominal_weight_kg_m = 0.007569f;

// avg belt length (vibrating part)
constexpr float length_belt = 0.267f;
// top belt length (vibrating part)
constexpr float length_top_belt = length_belt - 0.005f;
// bottom belt length (vibrating part)
constexpr float length_bottom_belt = length_belt + 0.005f;

// constants for calculating how many revolutions to do for any frequency differences
// this constants represents average frequency change of the belt per one revolution of its own screw
constexpr uint16_t belt_hz_per_rev = 15;
// this constants represents average frequency change of the belt per one revolution of second belt screw
constexpr uint16_t belt_hz_per_rev2 = 12;

// Calculates belt tension from their resonant frequency.
// returns tension (in Newtons)
// Formula taken from http://www.hyperphysics.gsu.edu/hbase/Waves/string.html
constexpr float freq_to_tension(float frequency_hz, float length_m) {
    return 4 * nominal_weight_kg_m * length_m * length_m * frequency_hz * frequency_hz;
}

// Calculates resonant frequency from the tension
// returns resonant frequency (in Hz)
constexpr float tension_to_freq(uint16_t tension_n, float length_m) {
    return sqrt(tension_n / (4 * nominal_weight_kg_m * length_m * length_m));
}

// calculate acceleration for desired frequency
constexpr float calc_accel(float freq) {
    // returns acceleration
    return accel_min + (freq - freq_min) * (accel_max - accel_min) / (freq_max - freq_min);
}

// calculate how many revolutions should user rotate the screws
constexpr float calc_revs_from_freq(float df1, float df2, float k1, float k2) {
    // df1 - first belt frequency difference (actual - optimal)
    // df2 - second belt frequency difference (actual - optimal)
    // k1 - first belt frequency constant (hz/revs)
    // k2 - second belt frequency constant (hz/revs)
    // returns requiered revs for first belt
    return ((df2 * k2) - (df1 * k1)) / ((k2 * k2) - (k1 * k1));
}

constexpr float floor_to_half(float val) {
    return static_cast<float>(static_cast<int>(val * 2)) / 2.0f;
}

// optimal belt tension for the printer
constexpr uint16_t tension_optimal_N = 19;

// optimal frequency for top belt
constexpr float freq_top_belt_optimal = floor_to_half(tension_to_freq(tension_optimal_N, length_top_belt));
// optimal frequency for bottom belt
constexpr float freq_bottom_belt_optimal = floor_to_half(tension_to_freq(tension_optimal_N, length_bottom_belt));

}; // namespace manual_belt_tuning
