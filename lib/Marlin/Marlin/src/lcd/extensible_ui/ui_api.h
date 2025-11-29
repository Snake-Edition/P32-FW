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
#pragma once

/************
 * ui_api.h *
 ************/

/****************************************************************************
 *   Written By Marcio Teixeira 2018 - Aleph Objects, Inc.                  *
 *                                                                          *
 *   This program is free software: you can redistribute it and/or modify   *
 *   it under the terms of the GNU General Public License as published by   *
 *   the Free Software Foundation, either version 3 of the License, or      *
 *   (at your option) any later version.                                    *
 *                                                                          *
 *   This program is distributed in the hope that it will be useful,        *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *   GNU General Public License for more details.                           *
 *                                                                          *
 *   To view a copy of the GNU General Public License, go to the following  *
 *   location: <http://www.gnu.org/licenses/>.                              *
 ****************************************************************************/

#include "../../inc/MarlinConfig.h"

#include <str_utils.hpp>

namespace ExtUI {

  // The ExtUI implementation can store up to this many bytes
  // in the EEPROM when the methods onStoreSettings and
  // onLoadSettings are called.

  static constexpr size_t eeprom_data_size = 48;

  enum axis_t     : uint8_t { X, Y, Z };
  enum extruder_t : uint8_t { E0, E1, E2, E3, E4, E5 };
  enum heater_t   : uint8_t { H0, H1, H2, H3, H4, H5, BED, CHAMBER };
  enum fan_t      : uint8_t { FAN0, FAN1, FAN2, FAN3, FAN4, FAN5 };

  constexpr uint8_t extruderCount = EXTRUDERS;
  constexpr uint8_t hotendCount   = HOTENDS;
  constexpr uint8_t fanCount      = FAN_COUNT;

#if HAS_LEVELING && HAS_MESH
  void onMeshUpdate(const uint8_t xpos, const uint8_t ypos, const float zval);
#endif

  /**
   * Event callback routines
   *
   * Should be declared by EXTENSIBLE_UI and will be called by Marlin
   */
  void onStartup();
  void onIdle();
  void onPlayTone(const uint16_t frequency, const uint16_t duration);
  void onPrintTimerStarted();
  void onPrintTimerPaused();
  void onPrintTimerStopped();
  void onFilamentRunout(const extruder_t extruder);
  void onUserConfirmRequired(const char * const msg);
  void onUserConfirmRequired_P(PGM_P const pstr);
  void onFactoryReset();
  void onStoreSettings(char *);
  void onLoadSettings(const char *);
  void onConfigurationStoreWritten(bool success);
  void onConfigurationStoreRead(bool success);
};
