#include "PrusaGcodeSuite.hpp"

#include "feature/automatic_chamber_vents/automatic_chamber_vents.hpp"
#include <Marlin/src/core/serial.h>
#include <Marlin/src/gcode/gcode.h>

/** \addtogroup G-Codes
 * @{
 */

/**
 *### M870: Control chamber vents.
 *
 * Open or close the vent grille.
 * This does not automatically retract before moving the head.
 *
 *#### Parameters
 * - `O` - Open intake
 * - `C` - Close intake
 */
void PrusaGcodeSuite::M870() {
    const bool open = parser.seen('O');
    const bool close = parser.seen('C');

    if (open && close) {
        SERIAL_ERROR_MSG("M870: Cannot specify both O and C");
    } else if (open) {
        if (!automatic_chamber_vents::open()) {
            SERIAL_ERROR_MSG("M870: Failed to open chamber vents");
        }
    } else if (close) {
        if (!automatic_chamber_vents::close()) {
            SERIAL_ERROR_MSG("M870: Failed to close chamber vents");
        }
    }
}

/** @}*/
