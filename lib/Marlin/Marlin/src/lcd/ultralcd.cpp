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

#include "../inc/MarlinConfigPre.h"

#ifdef LED_BACKLIGHT_TIMEOUT
  #include "../feature/leds/leds.h"
#endif

// All displays share the MarlinUI class
#if HAS_DISPLAY
  #include "../gcode/queue.h"
  #include "ultralcd.h"
  #include "fontutils.h"
  MarlinUI ui;
  #if ENABLED(EXTENSIBLE_UI)
    #define START_OF_UTF8_CHAR(C) (((C) & 0xC0u) != 0x80u)
  #endif
  #if ENABLED(HOST_ACTION_COMMANDS)
    #include "../feature/host_actions.h"
  #endif
#endif

#if ENABLED(EXTENSIBLE_UI)
  constexpr uint8_t MAX_MESSAGE_LENGTH = 63;
  uint8_t MarlinUI::alert_level; // = 0
  char MarlinUI::status_message[MAX_MESSAGE_LENGTH + 1];
#endif

#if ENABLED(LCD_SET_PROGRESS_MANUALLY)
  MarlinUI::progress_t MarlinUI::progress_override; // = 0
#endif

#if HAS_BUZZER
  #include "../libs/buzzer.h"
  #if ENABLED(PCA9632_BUZZER)
    #include "../feature/leds/pca9632.h"
  #endif
  void MarlinUI::buzz(const long duration, const uint16_t freq) {
    #if ENABLED(LCD_USE_I2C_BUZZER)
      lcd.buzz(duration, freq);
    #elif ENABLED(PCA9632_BUZZER)
      pca9632_buzz(duration, freq);
    #elif USE_BEEPER
      buzzer.tone(duration, freq);
    #endif
  }
#endif

#if HAS_DISPLAY

  #if ENABLED(EXTENSIBLE_UI)
    #include "extensible_ui/ui_api.h"
  #endif

  ////////////////////////////////////////////
  /////////////// Status Line ////////////////
  ////////////////////////////////////////////

  #if ENABLED(STATUS_MESSAGE_SCROLLING)
    void MarlinUI::advance_status_scroll() {
      // Advance by one UTF8 code-word
      if (status_scroll_offset < utf8_strlen(status_message))
        while (!START_OF_UTF8_CHAR(status_message[++status_scroll_offset]));
      else
        status_scroll_offset = 0;
    }
    char* MarlinUI::status_and_len(uint8_t &len) {
      char *out = status_message + status_scroll_offset;
      len = utf8_strlen(out);
      return out;
    }
  #endif

  void MarlinUI::finish_status(const bool persist) {

    #if !(ENABLED(LCD_PROGRESS_BAR) && (PROGRESS_MSG_EXPIRE > 0))
      UNUSED(persist);
    #endif

    #if ENABLED(LCD_PROGRESS_BAR)
      progress_bar_ms = millis();
      #if PROGRESS_MSG_EXPIRE > 0
        expire_status_ms = persist ? 0 : progress_bar_ms + PROGRESS_MSG_EXPIRE;
      #endif
    #endif

    #if ENABLED(EXTENSIBLE_UI)
      ExtUI::onStatusChanged(status_message);
    #endif
  }

  bool MarlinUI::has_status() { return (status_message[0] != '\0'); }

  void MarlinUI::set_status(const char * const message, const bool persist) {
    if (alert_level) return;

    // Here we have a problem. The message is encoded in UTF8, so
    // arbitrarily cutting it will be a problem. We MUST be sure
    // that there is no cutting in the middle of a multibyte character!

    // Get a pointer to the null terminator
    const char* pend = message + strlen(message);

    //  If length of supplied UTF8 string is greater than
    // our buffer size, start cutting whole UTF8 chars
    while ((pend - message) > MAX_MESSAGE_LENGTH) {
      --pend;
      while (!START_OF_UTF8_CHAR(*pend)) --pend;
    };

    // At this point, we have the proper cut point. Use it
    uint8_t maxLen = pend - message;
    strncpy(status_message, message, maxLen);
    status_message[maxLen] = '\0';

    finish_status(persist);
  }

  #include <stdarg.h>

  void MarlinUI::status_printf_P(const uint8_t level, PGM_P const fmt, ...) {
    if (level < alert_level) return;
    alert_level = level;
    va_list args;
    va_start(args, fmt);
    vsnprintf_P(status_message, MAX_MESSAGE_LENGTH, fmt, args);
    va_end(args);
    finish_status(level > 0);
  }

  void MarlinUI::set_status_P(PGM_P const message, int8_t level) {
    if (level < 0) level = alert_level = 0;
    if (level < alert_level) return;
    alert_level = level;

    // Since the message is encoded in UTF8 it must
    // only be cut on a character boundary.

    // Get a pointer to the null terminator
    PGM_P pend = message + strlen_P(message);

    // If length of supplied UTF8 string is greater than
    // the buffer size, start cutting whole UTF8 chars
    while ((pend - message) > MAX_MESSAGE_LENGTH) {
      --pend;
      while (!START_OF_UTF8_CHAR(pgm_read_byte(pend))) --pend;
    };

    // At this point, we have the proper cut point. Use it
    uint8_t maxLen = pend - message;
    strncpy_P(status_message, message, maxLen);
    status_message[maxLen] = '\0';

    finish_status(level > 0);
  }

  void MarlinUI::set_alert_status_P(PGM_P const message) {
    set_status_P(message, 1);
  }

  #include "../Marlin.h"
  #include "../module/printcounter.h"

  PGM_P print_paused = GET_TEXT(MSG_PRINT_PAUSED);

  /**
   * Reset the status message
   */
  void MarlinUI::reset_status() {
    PGM_P printing = GET_TEXT(MSG_PRINTING);
    #if SERVICE_INTERVAL_1 > 0
      static const char service1[] PROGMEM = { "> " SERVICE_NAME_1 "!" };
    #endif
    #if SERVICE_INTERVAL_2 > 0
      static const char service2[] PROGMEM = { "> " SERVICE_NAME_2 "!" };
    #endif
    #if SERVICE_INTERVAL_3 > 0
      static const char service3[] PROGMEM = { "> " SERVICE_NAME_3 "!" };
    #endif
    PGM_P msg = nullptr;
    if (printingIsPaused())
      msg = print_paused;
    else if (print_job_timer.isRunning())
      msg = printing;

    #if SERVICE_INTERVAL_1 > 0
      else if (print_job_timer.needsService(1)) msg = service1;
    #endif
    #if SERVICE_INTERVAL_2 > 0
      else if (print_job_timer.needsService(2)) msg = service2;
    #endif
    #if SERVICE_INTERVAL_3 > 0
      else if (print_job_timer.needsService(3)) msg = service3;
    #endif

    if(msg) {
      set_status_P(msg, -1);
    }
  }

  void MarlinUI::abort_print() {
    #ifdef ACTION_ON_CANCEL
      host_action_cancel();
    #endif
    #if ENABLED(HOST_PROMPT_SUPPORT)
      host_prompt_open(PROMPT_INFO, PSTR("UI Aborted"), PSTR("Dismiss"));
    #endif
    print_job_timer.stop();
    set_status_P(GET_TEXT(MSG_PRINT_ABORTED));
  }

  #if ENABLED(PARK_HEAD_ON_PAUSE)
    #include "../gcode/queue.h"
  #endif

  void MarlinUI::pause_print() {
    #if ENABLED(HOST_PROMPT_SUPPORT)
      host_prompt_open(PROMPT_PAUSE_RESUME, PSTR("UI Pause"), PSTR("Resume"));
    #endif

    set_status_P(print_paused);

    #if ENABLED(PARK_HEAD_ON_PAUSE)
      queue.inject_P(PSTR("M25 P\nM24"));
    #elif defined(ACTION_ON_PAUSE)
      host_action_pause();
    #endif
  }

  void MarlinUI::resume_print() {
    reset_status();
    #if ENABLED(PARK_HEAD_ON_PAUSE)
      wait_for_heatup = wait_for_user = false;
    #endif
    #ifdef ACTION_ON_RESUME
      host_action_resume();
    #endif
    print_job_timer.start(); // Also called by M24
  }

  #if HAS_PRINT_PROGRESS

    MarlinUI::progress_t MarlinUI::_get_progress() {
      #if ENABLED(LCD_SET_PROGRESS_MANUALLY)
        const progress_t p = progress_override & PROGRESS_MASK;
      #else
        constexpr progress_t p = 0;
      #endif
      return p;
    }

  #endif

#endif // HAS_DISPLAY
