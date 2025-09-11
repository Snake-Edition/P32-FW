#pragma once

#include <footer_line.hpp>
#include <client_fsm_types.h>
#include <window_frame.hpp>
#include <window_text.hpp>
#include <radio_button_fsm.hpp>

/**
 * Standard layout frame.
 * Contains:
 * - Centered title
 * - Centered text (alignment can be changed)
 * - A FSM radio
 */
class FramePrompt : public window_frame_t {

public:
    FramePrompt(window_t *parent, FSMAndPhase fsm_phase, const string_view_utf8 &txt_title, const string_view_utf8 &txt_info, Align_t info_alignment = Align_t::CenterTop());

    /**
     * Used by WithFooter<>
     * @param footer to add to vertical stack
     */
    void add_footer(FooterLine &footer); // Used by WithFooter<>

protected:
    window_text_t title;
    window_text_t info;
    RadioButtonFSM radio;
};
