#include "frame_qr_prompt.hpp"

#include <gui/frame_qr_layout.hpp>
#include <img_resources.hpp>
#include <guiconfig/wizard_config.hpp>
#include <find_error.hpp>
#include <buddy/unreachable.hpp>

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

FrameQRPrompt::FrameQRPrompt(window_frame_t *parent, FSMAndPhase fsm_phase, std::optional<ErrCode> (*error_code_mapper)(FSMAndPhase fsm_phase))
    : FrameQRPrompt(parent, fsm_phase, string_view_utf8::MakeNULLSTR(), nullptr) {

    // Extracting information: Phase -> corresponding error code -> message
    const auto err_code = error_code_mapper(fsm_phase);
    if (!err_code.has_value()) {
        BUDDY_UNREACHABLE(); // Some phases do not have corresponding error codes - they should not be called with this constructor
    }

    const auto err = find_error(err_code.value());

    info.SetText(_(err.err_text));

    // link's internal buffer is used instead of link_buffer
    link.set_error_code(err_code.value());

    qr.set_error_code(err.err_code);
}
