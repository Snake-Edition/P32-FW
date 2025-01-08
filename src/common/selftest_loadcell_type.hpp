/**
 * @file selftest_loadcell_type.hpp
 * @author Radek Vana
 * @brief selftest loadcell data to be passed between threads
 * @date 2021-09-29
 */

#pragma once

#include <common/fsm_base_types.hpp>
#include "selftest_sub_state.hpp"
#include <limits>

struct SelftestLoadcell_t {
    static constexpr uint8_t countdown_undef = 0x7f; // 7 bits for countdown
    uint8_t progress = 0;
    uint8_t countdown = countdown_undef;
    int16_t temperature = std::numeric_limits<int16_t>::min();
    bool pressed_too_soon = false;
    bool failed = false; // workaround just to pass it to main selftest

    static SelftestLoadcell_t from_phaseData(fsm::PhaseData new_data) {
        SelftestLoadcell_t r;
        r.Deserialize(new_data);
        return r;
    }

    constexpr fsm::PhaseData Serialize() const {
        fsm::PhaseData ret;
        ret[0] = progress;
        ret[1] = countdown & 0x7f; // 7 bits for countdown
        ret[1] |= pressed_too_soon ? 0x80 : 0x00; // 8th bit for pressed_too_soon
        ret[2] = temperature & 0xff;
        ret[3] = temperature >> 8;
        return ret;
    }

    constexpr void Deserialize(fsm::PhaseData new_data) {
        progress = new_data[0];
        countdown = new_data[1] & 0x7f; // 7 bits for countdown
        pressed_too_soon = (new_data[1] & 0x80) == 0x80; // 8th bit for pressed_too_soon
        temperature = new_data[2] | (new_data[3] << 8);
    }

    constexpr bool operator==(const SelftestLoadcell_t &other) const = default;
    constexpr bool operator!=(const SelftestLoadcell_t &other) const = default;

    void Pass() {
        progress = 100;
        countdown = countdown_undef;
        pressed_too_soon = false;
    }
    void Fail() {
        progress = 100;
        failed = true;
    } // don't touch countdown and pressed_too_soon
    void Abort() {} // currently not needed
};
