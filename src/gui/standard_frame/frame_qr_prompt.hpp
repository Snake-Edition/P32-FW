#pragma once

#include <client_fsm_types.h>

#include <qr.hpp>
#include <window_icon.hpp>
#include <window_frame.hpp>
#include <window_text.hpp>
#include <radio_button_fsm.hpp>
#include <text_error_url.hpp>
#include <error_codes.hpp>
#include <optional>

/**
 * Standard layout frame with a QR code.
 * Contains:
 * - Left aligned text
 * - QR code & link
 * - A FSM radio
 */
class FrameQRPrompt {

public:
    FrameQRPrompt(window_t *parent, FSMAndPhase fsm_phase, const string_view_utf8 &info_text, const char *qr_suffix);

    /** Takes info text and QR suffix from the error code.
     *  Construct the frame with \param error_code_mapper function (phase -> ErrCode), to extract useful information from ErrDesc related to given phase.
     */
    FrameQRPrompt(window_frame_t *parent, FSMAndPhase fsm_phase, std::optional<ErrCode> (*error_code_mapper)(FSMAndPhase fsm_phase));

protected:
    window_text_t info;
    TextErrorUrlWindow link;
    window_icon_t icon_phone;
    QRErrorUrlWindow qr;
    RadioButtonFSM radio;

    std::array<char, 32> link_buffer;
};
