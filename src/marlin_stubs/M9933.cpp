#include <module/planner.h>
#include <common/gcode/gcode_parser.hpp>
#include <feature/cork/tracker.hpp>

#include "PrusaGcodeSuite.hpp"

/** \addtogroup G-Codes
 * @{
 */

/**
 *### M9933: Internal Cork.
 *
 * Internal GCode
 *
 *#### Usage
 *
 *    9922 Ccookie
 *
 */
void PrusaGcodeSuite::M9933() {
    GCodeParser2 parser;
    if (!parser.parse_marlin_command()) {
        return;
    }

    if (auto cookie = parser.option<buddy::cork::Tracker::Cookie>('C'); cookie.has_value()) {
        planner.synchronize();
        buddy::cork::tracker.mark_done(*cookie);
    }
}
/** @}*/
