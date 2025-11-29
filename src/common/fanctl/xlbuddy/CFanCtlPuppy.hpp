#pragma once

#include <stdint.h>
#include "CFanCtlCommon.hpp"

class CFanCtlPuppy final : public CFanCtlCommon {
public:
    CFanCtlPuppy(uint8_t dwarf_nr, uint8_t fan_nr, bool is_autofan, uint16_t max_rpm)
        : CFanCtlCommon(0, max_rpm)
        , dwarf_nr(dwarf_nr)
        , fan_nr(fan_nr)
        , is_autofan(is_autofan) {
        reset_fan();
    }

    virtual void enter_selftest_mode() override;

    virtual void exit_selftest_mode() override;

    void reset_fan();

    virtual bool set_pwm(uint16_t pwm) override;

    virtual bool selftest_set_pwm(uint8_t pwm) override;

    virtual uint8_t get_pwm() const override;

    virtual uint16_t get_actual_rpm() const override;

    virtual bool get_rpm_is_ok() const override;

    virtual FanState get_state() const override;

    // Not used
    virtual uint16_t get_min_pwm() const override { return 0; }
    virtual bool get_rpm_measured() const override { return false; }
    virtual void tick() override {}

private:
    const uint8_t dwarf_nr;
    const uint8_t fan_nr;
    const bool is_autofan;
};
