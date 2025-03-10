#include <marlin_stubs/PrusaGcodeSuite.hpp>
#include <feature/chamber_filtration/chamber_filtration.hpp>

/**
 * ### M147 - Turns on the chamber filtration for the current print only (overrides fillament settings)
 * (the filtration has to be enabled in the settings)
 *
 * #### Usage
 *
 *     M147
 *
 * ### M148 - Turns off the chamber filtration for the current print only (overrides fillament settings)
 *
 * #### Usage
 *
 *     M148
 *
 */

void PrusaGcodeSuite::M147_148() {

    GCodeParser2 p;
    if (!p.parse_marlin_command()) {
        return;
    }

    buddy::chamber_filtration().set_needs_filtration(p.command().codenum == 147);
}
