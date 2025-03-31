/**
 * @file
 */
#include "../../lib/Marlin/Marlin/src/gcode/queue.h"
#include "PrusaGcodeSuite.hpp"

/** \addtogroup G-Codes
 * @{
 */

/**
 *### M999: Reset MCU <a href="https://reprap.org/wiki/G-code#M999:_Restart_after_being_stopped_by_error">M999: Restart after being stopped by error</a>
 *
 *#### Usage
 *
 *    M999
 */
void PrusaGcodeSuite::M999() {
    queue.ok_to_send();
    osDelay(1000);
    NVIC_SystemReset();
}

/** @}*/
