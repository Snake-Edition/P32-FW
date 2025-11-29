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
    static constexpr uint8_t countdown_undef = 0b11111;

    uint8_t progress : 7 = 0;
    bool ignore_noisy : 1 = false;
    uint8_t countdown : 5 = countdown_undef;
    bool loadcell_noisy : 1 = false;
    bool wrong_tap : 1 = false;
    bool failed : 1 = false; // workaround just to pass it to main selftest
    int16_t temperature = std::numeric_limits<int16_t>::min();

    static SelftestLoadcell_t from_phaseData(fsm::PhaseData new_data) {
        SelftestLoadcell_t r;
        r.Deserialize(new_data);
        return r;
    }

    constexpr fsm::PhaseData Serialize() const {
        fsm::PhaseData ret;
        memcpy(ret.data(), this, sizeof(SelftestLoadcell_t));
        return ret;
    }

    constexpr void Deserialize(fsm::PhaseData new_data) {
        memcpy(this, new_data.data(), sizeof(SelftestLoadcell_t));
    }

    constexpr bool operator==(const SelftestLoadcell_t &other) const = default;
    constexpr bool operator!=(const SelftestLoadcell_t &other) const = default;

    void Pass() {
        progress = 100;
        countdown = countdown_undef;
        wrong_tap = false;
        loadcell_noisy = false;
    }
    void Fail() {
        progress = 100;
        failed = true;
    } // don't touch countdown and wrong_tap
    void Abort() {} // currently not needed
};

static_assert(sizeof(SelftestLoadcell_t) <= std::tuple_size<fsm::PhaseData>());
