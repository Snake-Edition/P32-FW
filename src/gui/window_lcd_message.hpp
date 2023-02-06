#pragma once

#include "window_text.hpp"

#define LCD_MESSAGE_MAX_LEN 30
extern char lcd_message_text[LCD_MESSAGE_MAX_LEN + 1];

class WindowLCDMessage : public AddSuperWindow<window_text_t> {
    char last_lcd_message_text[LCD_MESSAGE_MAX_LEN + 1];

public:
    WindowLCDMessage(window_t *parent, Rect16 rect);

protected:
    void windowEvent(EventLock /*has private ctor*/, window_t *sender, GUI_event_t event, void *param) override;
};
