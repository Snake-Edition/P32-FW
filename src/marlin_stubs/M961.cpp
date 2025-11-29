#include "PrusaGcodeSuite.hpp"
#include <feature/manual_belt_tuning/manual_belt_tuning_wizard.hpp>

/**
 *### M961: Manual Belt tuning
 *
 * Measures belt resonant frequency to determine their tensioning force.
 */
void PrusaGcodeSuite::M961() {
    manual_belt_tuning::run_wizard();
}
