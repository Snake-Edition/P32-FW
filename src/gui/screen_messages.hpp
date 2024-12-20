#pragma once

#include "window_header.hpp"
#include "status_footer.hpp"
#include "window_term.hpp"
#include "screen.hpp"

struct screen_messages_data_t : public screen_t {
    window_header_t header;
    StatusFooter footer;
    window_term_t term;
    term_buff_t<20, 13> term_buff;
    uint32_t last_message_id = 0;

public:
    screen_messages_data_t();

protected:
    virtual void windowEvent(window_t *sender, GUI_event_t event, void *param) override;
};
