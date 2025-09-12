/**
 * @file xl_enclosure.cpp
 */

#include "xl_enclosure.hpp"
#include "timing.h"
#include "config_store/store_instance.hpp"
#include "fanctl.hpp"
#include "gcode_info.hpp"
#include "filament.hpp"
#include <ctime>
#include <tools_mapping.hpp>
#include <marlin_server.hpp>
#include <buddy/unreachable.hpp>
#include <option/xl_enclosure_support.h>
#include <option/has_chamber_filtration_api.h>
#include <feature/chamber_filtration/chamber_filtration.hpp>

static_assert(XL_ENCLOSURE_SUPPORT() && HAS_CHAMBER_FILTRATION_API());

Enclosure xl_enclosure;

Enclosure::Enclosure()
    : active_mode(EnclosureMode::Idle)
    , fan_presence_test_sec(0)
    , is_enabled_(config_store().xl_enclosure_enabled.get()) //
{
    last_sec = ticks_s();
}

void Enclosure::setEnabled(bool set) {
    if (is_enabled_ == set) {
        return;
    }

    is_enabled_ = set;
    config_store().xl_enclosure_enabled.set(is_enabled_);
    if (set) {
        buddy::chamber_filtration().set_backend(buddy::ChamberFiltrationBackend::xl_enclosure);
    }
}

void Enclosure::testFanPresence(uint32_t curr_tick) {
    static constexpr uint32_t fan_presence_test_period_sec = 3;
    if (curr_tick - fan_presence_test_sec >= fan_presence_test_period_sec) {
        if (Fans::enclosure().get_rpm_is_ok()) {
            setEnabled(true);
            active_mode = EnclosureMode::Active;
        } else {
            setEnabled(false);
            is_temp_valid_ = false;
            active_mode = EnclosureMode::Idle;
            marlin_server::set_warning(WarningType::EnclosureFanError);
        }
        fan_presence_test_sec = 0;
    }
}

std::optional<buddy::Temperature> Enclosure::getEnclosureTemperature() {
    if (const auto temp = active_dwarf_board_temp.load(); is_temp_valid_ && temp.has_value()) {
        static constexpr int32_t dwarf_board_temp_model_difference = -15; // °C
        return *temp + dwarf_board_temp_model_difference;
    }
    return std::nullopt;
}

void Enclosure::updateTempValidationTimer() {
    const auto print_state = marlin_vars().print_state.get();
    if (!marlin_server::is_printing_state(print_state) && !marlin_server::printer_paused_extended()) {
        // Reset the counter
        is_temp_valid_ = false;
    } else if (!is_temp_valid_) {
        // Printer is printing/pausing/paused/resuming and temp is not validated yet
        static constexpr uint32_t footer_temp_delay_s = 5 * 60;
        const auto print_dur = marlin_vars().print_duration.get();
        if (print_dur >= footer_temp_delay_s) {
            is_temp_valid_ = true;
        }
    }
}

PWM255 Enclosure::calculatePwm(int32_t MCU_modular_bed_temp) {

    if (isMCUOverheating(MCU_modular_bed_temp)) {
        // Override Fan pwm control
        // Overheating modular bed MCU has priority over active_mode
        return PWM255::from_percent(100);
    }

    switch (active_mode) {

    case EnclosureMode::Idle:
        return PWM255(0);

    case EnclosureMode::Test:
        return PWM255::from_percent(50);

    case EnclosureMode::Active:
        return buddy::chamber_filtration().output_pwm();
    }

    BUDDY_UNREACHABLE();
}

bool Enclosure::isMCUOverheating(int32_t MCU_modular_bed_temp) {
    static constexpr int32_t MB_MCU_maxtemp = 80; // °C
    static constexpr int32_t MB_MCU_safe_temp_treshold = 75; // °C

    if (MCU_modular_bed_temp > MB_MCU_maxtemp) {
        is_mcu_overheated_ = true;
    } else if (MCU_modular_bed_temp <= MB_MCU_safe_temp_treshold) {
        is_mcu_overheated_ = false;
    }

    return is_mcu_overheated_;
}

void Enclosure::loop(int32_t MCU_modular_bed_temp, int16_t dwarf_board_temp) {

    static constexpr uint32_t tick_delay_sec = 1;
    const uint32_t curr_sec = ticks_s();
    if (curr_sec - last_sec < tick_delay_sec) {
        return;
    }

    last_sec = curr_sec;
    active_dwarf_board_temp = dwarf_board_temp; // update actual temp of active dwarf board

    // Deactivated enclosure
    if (!isEnabled() && active_mode != EnclosureMode::Idle) {
        active_mode = EnclosureMode::Idle;
        is_temp_valid_ = false;
    }

    switch (active_mode) {
    case EnclosureMode::Idle:

        if (isEnabled()) {
            active_mode = EnclosureMode::Test;
            fan_presence_test_sec = curr_sec;
        }
        break;

    case EnclosureMode::Test:

        testFanPresence(curr_sec);
        break;

    case EnclosureMode::Active:

        // Update temperature validation timer (even during MCU Cooling)
        updateTempValidationTimer();
        break;
    }

    // Control Fan PWM
    PWM255 fan_pwm = calculatePwm(MCU_modular_bed_temp);
    Fans::enclosure().set_pwm(fan_pwm.value);
}
