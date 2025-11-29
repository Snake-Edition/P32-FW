#include "remote_bed.hpp"

#include <option/has_puppy_modularbed.h>

#if HAS_PUPPY_MODULARBED()
    #include <puppies/modular_bed.hpp>

float remote_bed::get_heater_current() {
    return buddy::puppies::modular_bed.get_heater_current();
}

uint16_t remote_bed::get_mcu_temperature() {
    return buddy::puppies::modular_bed.get_mcu_temperature();
}

void remote_bed::safe_state() {
    buddy::puppies::modular_bed.safe_state();
}

#endif
