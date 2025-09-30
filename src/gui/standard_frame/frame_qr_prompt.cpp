#include "frame_qr_prompt.hpp"

#include <gui/frame_qr_layout.hpp>
#include <img_resources.hpp>
#include <guiconfig/wizard_config.hpp>

FrameQRPrompt::FrameQRPrompt(window_t *parent, FSMAndPhase fsm_phase, const string_view_utf8 &info_text, const char *qr_suffix)
    : info(parent, FrameQRLayout::text_rect(), is_multiline::yes, is_closed_on_click_t::no, info_text)
    , link(parent, FrameQRLayout::link_rect())
    , icon_phone(parent, FrameQRLayout::phone_icon_rect(), &img::hand_qr_59x72)
    , qr(parent, FrameQRLayout::qrcode_rect())
    , radio(parent, WizardDefaults::RectRadioButton(0), fsm_phase) //
{
    StringBuilder(link_buffer)
        .append_string("prusa.io/")
        .append_string(qr_suffix);
    link.SetText(string_view_utf8::MakeRAM(link_buffer.data()));

    qr.get_string_builder()
        .append_string("https://prusa.io/qr-")
        .append_string(qr_suffix);

    CaptureNormalWindow(radio);
    static_cast<window_frame_t *>(parent)->CaptureNormalWindow(*this);
}
