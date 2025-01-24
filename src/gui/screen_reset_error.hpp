// screen_reset_perror.hpp
#pragma once
#include "gui.hpp"
#include "window_text.hpp"
#include "screen.hpp"

/**
 * @brief Generic screen shown after reset caused by error.
 * Reads error details from dump.
 */
class ScreenResetError : public screen_t {
    bool sound_started;
    std::array<char, 42> fw_version_str;

public:
    ScreenResetError(const Rect16 &fw_version_rect);

protected:
    window_text_t fw_version_txt;
    /// starts sound and avoids repetitive starting
    void start_sound();
    virtual void windowEvent(window_t *sender, GUI_event_t event, void *param) override;
};
