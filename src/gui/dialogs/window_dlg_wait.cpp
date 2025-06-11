/*
 * window_dlg_wait.c
 *
 *  Created on: Nov 5, 2019
 *      Author: Migi
 */
#include "window_dlg_wait.hpp"
#include "gui.hpp"
#include "i18n.h"
#include "ScreenHandler.hpp"
#include <DialogHandler.hpp>

static const constexpr uint16_t ANIMATION_MILISEC_DELAY = 500; // number of milisecond for frame change
static const constexpr int animation_y = 130; // animation anchor point on Y axis
static const constexpr int animation_x = GuiDefaults::EnableDialogBigLayout ? 220 : 110; // animation anchor point on X axis
static const constexpr int text_y_offset = GuiDefaults::EnableDialogBigLayout ? 30 : 10; // text point on y axis
static const constexpr int second_text_y_offset = GuiDefaults::EnableDialogBigLayout ? 67 : 45; // text point on y axis

static constexpr EnumArray<PhaseWait, const char *, PhaseWait::_cnt> phase_texts {
    { PhaseWait::generic, nullptr },
    { PhaseWait::homing, N_("Printer may vibrate and be noisier during homing.") },
    { PhaseWait::homing_calibration, N_("Recalibrating home. This may take some time.") },
};

window_dlg_wait_t::window_dlg_wait_t(Rect16 rect, const string_view_utf8 &second_text_string)
    : IDialogMarlin(rect)
    , text(this, { rect.Left(), int16_t(rect.Top() + text_y_offset), rect.Width(), uint16_t(30) }, is_multiline::no, is_closed_on_click_t::no, _("Please wait"))
    , second_text(this, { int16_t(rect.Left() + GuiDefaults::FramePadding), int16_t(rect.Top() + second_text_y_offset), uint16_t(rect.Width() - 2 * GuiDefaults::FramePadding), uint16_t(60) }, is_multiline::yes, is_closed_on_click_t::no, second_text_string)
    , animation(this, { int16_t(rect.Left() + animation_x), int16_t(rect.Top() + animation_y) }) {
    text.set_font(GuiDefaults::FontBig);
    text.SetAlignment(Align_t::Center());

    second_text.set_font(GuiDefaults::FooterFont);
    second_text.SetAlignment(Align_t::Center());
}

window_dlg_wait_t::window_dlg_wait_t(fsm::BaseData data)
    : window_dlg_wait_t(_(phase_texts[data.GetPhase()])) {}

void window_dlg_wait_t::Change(fsm::BaseData data) {
    second_text.SetText(_(phase_texts[data.GetPhase()]));
}

void window_dlg_wait_t::wait_for_gcodes_to_finish() {
    // If the wait is short enough, don't show the wait dialog - it would just blink the screen
    for (int i = 0; i < 5; i++) {
        if (!marlin_vars().is_processing.get()) {
            return;
        }
        osDelay(10);
    }

    // Then show a wait dialog
    window_dlg_wait_t dlg(string_view_utf8 {});
    Screens::Access()->gui_loop_until_dialog_closed([&] {
        if (!marlin_vars().is_processing.get()) {
            Screens::Access()->Close();
            return;
        }

        // This one is important - it allows popping up a warning dialog on top of this one
        DialogHandler::Access().Loop();
    });
}

void gui_dlg_wait(stdext::inplace_function<void()> closing_callback, const string_view_utf8 &second_string) {
    window_dlg_wait_t dlg(second_string);
    Screens::Access()->gui_loop_until_dialog_closed(closing_callback);
}
