#include "text_roll.hpp"
#include <algorithm>

#include "display_helper.h"
#include "display.hpp"
#include "window.hpp"
#include "gui.hpp"
#include "../lang/string_view_utf8.hpp"
#include "../common/str_utils.hpp"
#include "ScreenHandler.hpp"
#include <common/conserve_cpu.hpp>

size_t txtroll_t::instance_counter = 0;

// invalidate at phase change
invalidate_t txtroll_t::Tick() {
    if (buddy::conserve_cpu().is_requested()) {
        // We are being asked to limit CPU, so we won't do rolling for a while.
        switch (phase) {
        case phase_t::uninitialized:
        case phase_t::idle:
        case phase_t::paused:
        case phase_t::wait_before_roll:
            return invalidate_t::no;
        default:
            phase = phase_t::init_roll;
            break;
        }
    }

    invalidate_t ret = invalidate_t::no;
    switch (phase) {
    case phase_t::uninitialized:
    case phase_t::idle:
    case phase_t::paused:
        break;
    case phase_t::init_roll:
        px_cd = 0;
        count = hidden_char_cnt;
        phase = hidden_char_cnt == 0 ? phase_t::idle : phase_t::wait_before_roll;
        ret = invalidate_t::yes;
        phase_progress = (wait_before_roll_ms + base_tick_ms - 1) / base_tick_ms;
        break;
    case phase_t::wait_before_roll:
        if ((--phase_progress) == 0) {
            phase = phase_t::go;
            ret = invalidate_t::yes;
            draw_progress = 0;
        }
        break;
    case phase_t::go:
        if (count > 0 || px_cd > 0) {
            if (px_cd == 0) {
                px_cd = font_w;
                count--;
                draw_progress++;
            }
            px_cd--;
        } else {
            phase = phase_t::wait_after_roll;
            phase_progress = (wait_after_roll_ms + base_tick_ms - 1) / base_tick_ms;
        }
        ret = invalidate_t::yes;
        break;
    case phase_t::wait_after_roll:
        if ((--phase_progress) == 0) {
            phase = phase_t::init_roll;
            ret = invalidate_t::yes;
        }
        break;
    }

    return ret;
}

static uint8_t runtime_width(Font font) {
    return resource_font(font)->w;
}

void txtroll_t::Init(Rect16 rc, const string_view_utf8 &text, Font font,
    padding_ui8_t padding, Align_t alignment) {
    rect = rect_meas(rc, text, font, padding, alignment);
    hidden_char_cnt = meas(rect, text, font);
    font_w = runtime_width(font);
    phase = phase_t::init_roll;
}

void txtroll_t::render_text(Rect16 rc, const string_view_utf8 &text, Font font,
    Color clr_back, Color clr_text, padding_ui8_t padding, Align_t alignment, bool fill_rect) const {
    switch (phase) {
    case phase_t::uninitialized:
    case phase_t::idle:
    case phase_t::init_roll:
    case phase_t::wait_before_roll:
        render_text_align(rc, text, font, clr_back, clr_text, padding, text_flags(alignment, is_multiline::no), fill_rect);
        break;
    default: {
        Rect16 text_rect = Rect16(rect.Left() + px_cd, rect.Top(), (rect.Width() - px_cd) - ((rect.Width() - px_cd) % runtime_width(font)), rect.Height()); // Align width to be multiplicand of font width (for correct fill_rect)
        auto reader = StringReaderUtf8(text);
        reader.skip(draw_progress);
        render_text_align(text_rect, StringReaderUtf8(text).skip(draw_progress), font, clr_back, clr_text, padding, alignment, fill_rect);
    } break;
    }
}

Rect16 txtroll_t::rect_meas(Rect16 rc, const string_view_utf8 &text, Font font, padding_ui8_t padding, Align_t alignment) {

    Rect16 rc_pad = rc;
    rc_pad.CutPadding(padding);
    const auto pf = resource_font(font);
    StringReaderUtf8 reader(text);
    const auto layout = RectTextLayout(reader, rc.Width() / pf->w, 1, is_multiline::no);
    size_ui16_t txt_size = size_ui16_t(layout.get_width_in_chars() * pf->w, pf->h);
    Rect16 rc_txt = { 0, 0, 0, 0 };
    if (txt_size.w && txt_size.h) {
        rc_txt = Rect16(0, 0, txt_size.w, txt_size.h);
        rc_txt.Align(rc_pad, alignment);
        rc_txt = rc_txt.Intersection(rc_pad);
    }
    return rc_txt;
}

uint16_t txtroll_t::meas(Rect16 rc, const string_view_utf8 &text, Font pf) {
    uint16_t meas_x = 0, len = text.computeNumUtf8Chars();
    if (len * runtime_width(pf) > rc.Width()) {
        meas_x = len - rc.Width() / runtime_width(pf);
    }
    return meas_x;
}
