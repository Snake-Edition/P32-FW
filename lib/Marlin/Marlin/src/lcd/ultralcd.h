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

#include "../inc/MarlinConfig.h"
#include <option/has_gui.h>

#if HAS_BUZZER
  #include "../libs/buzzer.h"
#endif

#define HAS_DIGITAL_BUTTONS (BUTTONS_EXIST(EN1, EN2) || ANY_BUTTON(ENC, BACK, UP, DWN, LFT, RT))
#define HAS_SHIFT_ENCODER   (!HAS_ADC_BUTTONS && (ENABLED(REPRAPWORLD_KEYPAD)))
#define HAS_ENCODER_WHEEL   (BUTTONS_EXIST(EN1, EN2))

// I2C buttons must be read in the main thread
#define HAS_SLOW_BUTTONS EITHER(LCD_I2C_VIKI, LCD_I2C_PANELOLU2)

// REPRAPWORLD_KEYPAD (and ADC_KEYPAD)
#if ENABLED(REPRAPWORLD_KEYPAD)
  #define BTN_OFFSET          0 // Bit offset into buttons for shift register values

  #define BLEN_KEYPAD_F3      0
  #define BLEN_KEYPAD_F2      1
  #define BLEN_KEYPAD_F1      2
  #define BLEN_KEYPAD_DOWN    3
  #define BLEN_KEYPAD_RIGHT   4
  #define BLEN_KEYPAD_MIDDLE  5
  #define BLEN_KEYPAD_UP      6
  #define BLEN_KEYPAD_LEFT    7

  #define EN_KEYPAD_F1      _BV(BTN_OFFSET + BLEN_KEYPAD_F1)
  #define EN_KEYPAD_F2      _BV(BTN_OFFSET + BLEN_KEYPAD_F2)
  #define EN_KEYPAD_F3      _BV(BTN_OFFSET + BLEN_KEYPAD_F3)
  #define EN_KEYPAD_DOWN    _BV(BTN_OFFSET + BLEN_KEYPAD_DOWN)
  #define EN_KEYPAD_RIGHT   _BV(BTN_OFFSET + BLEN_KEYPAD_RIGHT)
  #define EN_KEYPAD_MIDDLE  _BV(BTN_OFFSET + BLEN_KEYPAD_MIDDLE)
  #define EN_KEYPAD_UP      _BV(BTN_OFFSET + BLEN_KEYPAD_UP)
  #define EN_KEYPAD_LEFT    _BV(BTN_OFFSET + BLEN_KEYPAD_LEFT)

  #define RRK(B) (keypad_buttons & (B))

  #ifdef EN_C
    #define BUTTON_CLICK() ((buttons & EN_C) || RRK(EN_KEYPAD_MIDDLE))
  #else
    #define BUTTON_CLICK() RRK(EN_KEYPAD_MIDDLE)
  #endif

#endif

#if HAS_DIGITAL_BUTTONS

  // Wheel spin pins where BA is 00, 10, 11, 01 (1 bit always changes)
  #define BLEN_A 0
  #define BLEN_B 1

  #define EN_A _BV(BLEN_A)
  #define EN_B _BV(BLEN_B)

  #define BUTTON_PRESSED(BN) !READ(BTN_## BN)

  #if BUTTON_EXISTS(ENC)
    #define BLEN_C 2
    #define EN_C _BV(BLEN_C)
  #endif

  #if ENABLED(LCD_I2C_VIKI)

    #define B_I2C_BTN_OFFSET 3 // (the first three bit positions reserved for EN_A, EN_B, EN_C)

    // button and encoder bit positions within 'buttons'
    #define B_LE (BUTTON_LEFT   << B_I2C_BTN_OFFSET)      // The remaining normalized buttons are all read via I2C
    #define B_UP (BUTTON_UP     << B_I2C_BTN_OFFSET)
    #define B_MI (BUTTON_SELECT << B_I2C_BTN_OFFSET)
    #define B_DW (BUTTON_DOWN   << B_I2C_BTN_OFFSET)
    #define B_RI (BUTTON_RIGHT  << B_I2C_BTN_OFFSET)

    #if BUTTON_EXISTS(ENC)                                // The pause/stop/restart button is connected to BTN_ENC when used
      #define B_ST (EN_C)                                 // Map the pause/stop/resume button into its normalized functional name
      #define BUTTON_CLICK() (buttons & (B_MI|B_RI|B_ST)) // Pause/stop also acts as click until a proper pause/stop is implemented.
    #else
      #define BUTTON_CLICK() (buttons & (B_MI|B_RI))
    #endif

    // I2C buttons take too long to read inside an interrupt context and so we read them during lcd_update

  #elif ENABLED(LCD_I2C_PANELOLU2)

    #if !BUTTON_EXISTS(ENC) // Use I2C if not directly connected to a pin

      #define B_I2C_BTN_OFFSET 3 // (the first three bit positions reserved for EN_A, EN_B, EN_C)

      #define B_MI (PANELOLU2_ENCODER_C << B_I2C_BTN_OFFSET) // requires LiquidTWI2 library v1.2.3 or later

      #define BUTTON_CLICK() (buttons & B_MI)

    #endif

  #endif

#else

  #undef BUTTON_EXISTS
  #define BUTTON_EXISTS(...) false

  // Shift register bits correspond to buttons:
  #define BL_LE 7   // Left
  #define BL_UP 6   // Up
  #define BL_MI 5   // Middle
  #define BL_DW 4   // Down
  #define BL_RI 3   // Right
  #define BL_ST 2   // Red Button
  #define B_LE (_BV(BL_LE))
  #define B_UP (_BV(BL_UP))
  #define B_MI (_BV(BL_MI))
  #define B_DW (_BV(BL_DW))
  #define B_RI (_BV(BL_RI))
  #define B_ST (_BV(BL_ST))

  #ifndef BUTTON_CLICK
    #define BUTTON_CLICK() (buttons & (B_MI|B_ST))
  #endif

#endif

#if BUTTON_EXISTS(BACK)
  #define BLEN_D 3
  #define EN_D _BV(BLEN_D)
  #define LCD_BACK_CLICKED() (buttons & EN_D)
#else
  #define LCD_BACK_CLICKED() false
#endif

#ifndef BUTTON_CLICK
  #ifdef EN_C
    #define BUTTON_CLICK() (buttons & EN_C)
  #else
    #define BUTTON_CLICK() false
  #endif
#endif

////////////////////////////////////////////
//////////// MarlinUI Singleton ////////////
////////////////////////////////////////////

class MarlinUI {
public:

  MarlinUI() {
  }

  #if HAS_BUZZER
    static void buzz(const long duration, const uint16_t freq);
  #endif

  #if ENABLED(LCD_HAS_STATUS_INDICATORS)
    static void update_indicators();
  #endif

  static void init();
  static void update();
  static void abort_print();
  static void pause_print();
  static void resume_print();

  #if HAS_GUI()

    static void set_alert_status_P(PGM_P const message);

    static char status_message[];
    static bool has_status();

    static uint8_t alert_level; // Higher levels block lower levels
    static inline void reset_alert_level() { alert_level = 0; }

    #if ENABLED(STATUS_MESSAGE_SCROLLING)
      static uint8_t status_scroll_offset;
      static void advance_status_scroll();
      static char* status_and_len(uint8_t &len);
    #endif

    #if HAS_PRINT_PROGRESS
      #if HAS_PRINT_PROGRESS_PERMYRIAD
        typedef uint16_t progress_t;
        #define PROGRESS_SCALE 100U
        #define PROGRESS_MASK 0x7FFF
      #else
        typedef uint8_t progress_t;
        #define PROGRESS_SCALE 1U
        #define PROGRESS_MASK 0x7F
      #endif
      #if ENABLED(LCD_SET_PROGRESS_MANUALLY)
        static progress_t progress_override;
        static void set_progress(const progress_t p) { progress_override = _MIN(p, 100U * (PROGRESS_SCALE)); }
        static void set_progress_done() { progress_override = (PROGRESS_MASK + 1U) + 100U * (PROGRESS_SCALE); }
        static void progress_reset() { if (progress_override & (PROGRESS_MASK + 1U)) set_progress(0); }
      #endif
      static progress_t _get_progress();
      #if HAS_PRINT_PROGRESS_PERMYRIAD
        static uint16_t get_progress_permyriad() { return _get_progress(); }
      #endif
      static uint8_t get_progress_percent() { return uint8_t(_get_progress() / (PROGRESS_SCALE)); }
    #else
      static constexpr uint8_t get_progress_percent() { return 0; }
    #endif

    static void refresh() {}

    static bool get_blink();
    static void set_status(const char* const message, const bool persist=false);
    static void set_status_P(PGM_P const message, const int8_t level=0);
    static void status_printf_P(const uint8_t level, PGM_P const fmt, ...);
    static void reset_status();

  #else // HAS_GUI()

    static inline void refresh() {}
    static inline void return_to_status() {}
    static inline void set_alert_status_P(PGM_P const) {}
    static inline void set_status(const char* const, const bool=false) {}
    static inline void set_status_P(PGM_P const, const int8_t=0) {}
    static inline void status_printf_P(const uint8_t, PGM_P const, ...) {}
    static inline void reset_status() {}
    static inline void reset_alert_level() {}
    static constexpr bool has_status() { return false; }
    static constexpr uint8_t get_progress_percent() { return 0; }

  #endif

  #if ENABLED(LCD_BED_LEVELING) && ENABLED(PROBE_MANUALLY)
    static bool wait_for_bl_move;
  #else
    static constexpr bool wait_for_bl_move = false;
  #endif

  static constexpr bool external_control = false;

  static inline void update_buttons() {}

private:

  static void _synchronize();

  #if HAS_DISPLAY
    static void finish_status(const bool persist);
  #endif
};

extern MarlinUI ui;

#define LCD_MESSAGEPGM_P(x)      ui.set_status_P(x)
#define LCD_ALERTMESSAGEPGM_P(x) ui.set_alert_status_P(x)

#define LCD_MESSAGE(x)           LCD_MESSAGEPGM_P(GET_TEXT(x))
#define LCD_MESSAGEPGM(x)        LCD_MESSAGEPGM_P(GET_TEXT(x))
#define LCD_ALERTMESSAGEPGM(x)   LCD_ALERTMESSAGEPGM_P(GET_TEXT(x))
