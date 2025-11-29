/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2019 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/**
 * stepper/indirection.cpp
 *
 * Stepper motor driver indirection to allow some stepper functions to
 * be done via SPI/I2c instead of direct pin manipulation.
 *
 * Copyright (c) 2015 Dominik Wenger
 */

#include "indirection.h"
#include <bsod.h>

void stepper_enable(AxisEnum axis, bool enabled) {
    switch (axis) {
#if ENABLED(XY_LINKED_ENABLE)
    case X_AXIS:
    case Y_AXIS:
        if (enabled) {
            enable_XY();
        } else {
            disable_XY();
        }
        break;
#else
    #if !SAME_DRIVER_ID(X_DRIVER_TYPE, NONE)
    case X_AXIS:
        if (enabled) {
            enable_X();
        } else {
            disable_X();
        }
        break;
    #endif
    #if !SAME_DRIVER_ID(Y_DRIVER_TYPE, NONE)
    case Y_AXIS:
        if (enabled) {
            enable_Y();
        } else {
            disable_Y();
        }
        break;
    #endif
#endif
#if !SAME_DRIVER_ID(Z_DRIVER_TYPE, NONE)
    case Z_AXIS:
        if (enabled) {
            enable_Z();
        } else {
            disable_Z();
        }
        break;
#endif
#if E_STEPPERS > 0
    case E0_AXIS:
        if (enabled) {
            enable_E0();
        } else {
            disable_E0();
        }
        break;
#endif
#if E_STEPPERS > 1
    case E1_AXIS:
        if (enabled) {
            enable_E1();
        } else {
            disable_E1();
        }
        break;
#endif
#if E_STEPPERS > 2
    case E2_AXIS:
        if (enabled) {
            enable_E2();
        } else {
            disable_E2();
        }
        break;
#endif
#if E_STEPPERS > 3
    case E3_AXIS:
        if (enabled) {
            enable_E3();
        } else {
            disable_E3();
        }
        break;
#endif
#if E_STEPPERS > 4
    case E4_AXIS:
        if (enabled) {
            enable_E4();
        } else {
            disable_E4();
        }
        break;
#endif
#if E_STEPPERS > 5
    case E5_AXIS:
        if (enabled) {
            enable_E5();
        } else {
            disable_E5();
        }
        break;
#endif
    default:
        bsod("invalid stepper axis");
    }
}

bool stepper_enabled(AxisEnum axis) {
    switch (axis) {
#if ENABLED(XY_LINKED_ENABLE)
    case X_AXIS:
    case Y_AXIS:
        return X_ENABLE_READ();
#else
    #if !SAME_DRIVER_ID(X_DRIVER_TYPE, NONE)
    case X_AXIS:
        return X_ENABLE_READ();
    #endif
    #if !SAME_DRIVER_ID(Y_DRIVER_TYPE, NONE)
    case Y_AXIS:
        return Y_ENABLE_READ();
    #endif
#endif
#if !SAME_DRIVER_ID(Z_DRIVER_TYPE, NONE)
    case Z_AXIS:
        return Z_ENABLE_READ();
#endif
#if E_STEPPERS > 0
    case E0_AXIS:
        return E0_ENABLE_READ();
#endif
#if E_STEPPERS > 1
    case E1_AXIS:
        return E1_ENABLE_READ();
#endif
#if E_STEPPERS > 2
    case E2_AXIS:
        return E2_ENABLE_READ();
#endif
#if E_STEPPERS > 3
    case E3_AXIS:
        return E3_ENABLE_READ();
#endif
#if E_STEPPERS > 4
    case E4_AXIS:
        return E4_ENABLE_READ();
#endif
#if E_STEPPERS > 5
    case E5_AXIS:
        return E5_ENABLE_READ();
#endif
    default:
        bsod("invalid stepper axis");
    }
}
