#include "screen_belt_tuning.hpp"
#include <feature/manual_belt_tuning/manual_belt_tuning_config.hpp>

#include <marlin_server_types/fsm/manual_belt_tuning_phases.hpp>
#include <img_resources.hpp>
#include <guiconfig/wizard_config.hpp>
#include <window_numb.hpp>
#include <client_response.hpp>
#include <qr.hpp>
#include <meta_utils.hpp>

using Phase = PhaseManualBeltTuning;

namespace {

// Introduction to belt tuning calibration, QR, prerequisites PHASE: intro
constexpr const char link_begin_calib[] = "prusa.io/core-belt-calibration";
constexpr const char *txt_title_begin = N_("Let's calibrate your belt tension");
constexpr const char *txt_desc_begin = N_("Before we begin, scan the QR code for the guide. You'll also need an Allen key.");
// Checking if X-axis gantry is correctly lined up PHASE: check_x_gantry
constexpr const char link_belt_calib_gantry[] = "prusa.io/core-belt-calibration-gantry";
constexpr const char *txt_title_gantry = N_("Check X-axis gantry squareness");
constexpr const char *txt_desc_gantry = N_("Move printhead to front. Check there's no gap between gantry and tensioner on both ends. If there is, follow the guide.");
// Waiting for homing and moving to wizard position
constexpr const char *txt_wait = N_("Printer is homing, please wait.");
// Prepare for measuring actual frequencies, QR, prerequisites PHASE: intro_measure
constexpr const char *txt_title_measure = N_("Measuring actual belt frequency");
constexpr const char *txt_desc_measure = N_("The upper belt will be measured first, then the lower. Use the knob to adjust resonance. Correct frequency appears as slow belt motion with sharp peaks.");
// Measuring actual upper belt frequency, Knob PHASE: measure_upper_belt
constexpr const char *txt_title_up_belt_freq = N_("Upper belt actual frequency");
constexpr const char *txt_desc_up_belt_freq = N_("Turn the knob to adjust frequency. Look for slow belt movement with sharp, regular peaks, then click the knob to proceed.");
// Measuring actual lower belt frequency, Knob PHASE: measure_lower_belt
constexpr const char *txt_title_lo_belt_freq = N_("Lower belt actual frequency");
constexpr const char *txt_desc_lo_belt_freq = N_("Turn the knob to adjust frequency. Look for slow belt movement with sharp, regular peaks, then click the knob to proceed.");
// Show results of both belt measurements in Newtons PHASE: show_tension
constexpr const char *txt_title_freq_report = N_("Actual belts frequencies");
constexpr const char *txt_desc_freq_report = N_("Upper belt: %.1f Hz\nOptimal tension is between %u and %u Hz\n\nLower belt: %.1f Hz\nOptimal tension is between %u and %u Hz\n\nReady to calculate screw adjustment.\nPress Continue to proceed.");
// Warning about x-gantry alignment issue PHASE: alignment_issue
constexpr const char *txt_title_alignment_issue = N_("Alignment issue detected");
constexpr const char *txt_desc_alignment_issue = N_("Measured belt frequencies are out of the expected range.\nPlease check that the gantry is aligned properly and repeat the calibration.");
// User adjusts the tensioners with calculated allen key turns PHASE: adjust_tensioners
constexpr const char link_tensioning[] = "prusa.io/core-belt-calibration-tensioning";
constexpr const char *txt_tighten = N_("tighten");
constexpr const char *txt_loosen = N_("loosen");
constexpr const char *txt_title_turn_screw = N_("Adjust belt tensioners");
constexpr const char *txt_desc_turn_screw = N_("Turn both screws:\n%s %s turns\n\nPress Continue when done.");
constexpr const char *txt_desc_no_turn = N_("No need to turn the tensioner screws.\n\nPress Continue to finish calibration or Adjust to repeat measuring.");
// Finish screen
constexpr const char *txt_title_finished = N_("Calibration complete");
constexpr const char *txt_desc_finished = N_("Belt tension has been successfully calibrated.\nYou're all set and ready to print.\n\nPress Finish to exit.");

constexpr uint8_t qr_size = 100;

constexpr Rect16 rect_title = Rect16(WizardDefaults::MarginLeft, WizardDefaults::row_0, GuiDefaults::ScreenWidth - WizardDefaults::MarginLeft - WizardDefaults::MarginRight, WizardDefaults::txt_h);
constexpr Rect16 rect_line = Rect16(WizardDefaults::MarginLeft, WizardDefaults::row_1, GuiDefaults::ScreenWidth - WizardDefaults::MarginLeft - WizardDefaults::MarginRight, 1);

constexpr Rect16 rect_hourglass = Rect16(WizardDefaults::MarginLeft, WizardDefaults::row_0, GuiDefaults::ScreenWidth - WizardDefaults::MarginLeft - WizardDefaults::MarginRight, 110);
constexpr Rect16 rect_wait = Rect16(WizardDefaults::MarginLeft, rect_hourglass.Bottom() + WizardDefaults::txt_h, GuiDefaults::ScreenWidth - WizardDefaults::MarginLeft - WizardDefaults::MarginRight, WizardDefaults::txt_h);

constexpr Rect16 rect_qr = Rect16(GuiDefaults::ScreenWidth - WizardDefaults::MarginRight - qr_size, WizardDefaults::row_1 + 10 /*=visual delimeter*/, qr_size, qr_size);
constexpr Rect16 rect_desc = Rect16(WizardDefaults::MarginLeft, WizardDefaults::row_1 + 10 /*=visual delimeter*/, GuiDefaults::ScreenWidth - WizardDefaults::MarginLeft - WizardDefaults::MarginRight, WizardDefaults::Y_space - WizardDefaults::RectRadioButton(0).Height() - WizardDefaults::row_h - 30 /*=visual space*/);
constexpr Rect16 rect_desc_qr = Rect16(WizardDefaults::MarginLeft, WizardDefaults::row_1 + 10 /*=visual delimeter*/, GuiDefaults::ScreenWidth - WizardDefaults::MarginLeft - WizardDefaults::MarginRight - qr_size - 5 /*=padding*/, WizardDefaults::Y_space - WizardDefaults::RectRadioButton(0).Height() - WizardDefaults::row_h - 70 /*=space for link*/);

constexpr Rect16 rect_scan_me = Rect16(rect_qr.Left(), rect_qr.Bottom(), qr_size, WizardDefaults::txt_h);
constexpr Rect16 rect_details = Rect16(WizardDefaults::MarginLeft, rect_desc_qr.Bottom(), GuiDefaults::ScreenWidth, WizardDefaults::txt_h);
constexpr Rect16 rect_link = Rect16(WizardDefaults::MarginLeft, rect_details.Bottom(), GuiDefaults::ScreenWidth, WizardDefaults::txt_h);
constexpr Rect16 rect_desc_knob = Rect16(WizardDefaults::MarginLeft, WizardDefaults::row_1 + 10, GuiDefaults::ScreenWidth - WizardDefaults::MarginLeft - WizardDefaults::MarginRight, WizardDefaults::Y_space - WizardDefaults::RectRadioButton(0).Height() - WizardDefaults::row_h - 80 /*=visual space*/);

constexpr Rect16 rect_numb = Rect16(GuiDefaults::ScreenWidth / 2 - 50, WizardDefaults::RectRadioButton(0).Top() - 100, 100, 22);
constexpr Rect16 rect_knob = Rect16(GuiDefaults::ScreenWidth / 2 - 41, WizardDefaults::RectRadioButton(0).Top() - 70, 81, 55);
constexpr Rect16 rect_minus = Rect16(GuiDefaults::ScreenWidth / 2 - 61, WizardDefaults::RectRadioButton(0).Top() - 70, 20, 55);
constexpr Rect16 rect_plus = Rect16(GuiDefaults::ScreenWidth / 2 + 40, WizardDefaults::RectRadioButton(0).Top() - 70, 20, 55);

constexpr Rect16 rect_text_joe = Rect16(WizardDefaults::MarginLeft, WizardDefaults::row_1 + 10, GuiDefaults::ScreenWidth - WizardDefaults::MarginLeft - WizardDefaults::MarginRight - 50, WizardDefaults::Y_space - WizardDefaults::RectRadioButton(0).Height() - WizardDefaults::row_h - 30 /*=visual space*/ - 64 /* icon */);
constexpr Rect16 rect_joe = Rect16(0, rect_text_joe.Bottom(), GuiDefaults::ScreenWidth, 64);
} // namespace

namespace frames {

class FrameWait : public window_frame_t {
public:
    FrameWait(window_t *parent)
        : window_frame_t(parent, parent->GetRect())
        , hourglass(this, rect_hourglass, &img::hourglass_26x39)
        , wait(this, rect_wait, is_multiline::no, is_closed_on_click_t::no, _(txt_wait)) {
        hourglass.SetAlignment(Align_t::CenterBottom());
        wait.SetAlignment(Align_t::Center());
    }

protected:
    window_icon_t hourglass;
    window_text_t wait;
};

class FrameTitle : public window_frame_t {
public:
    FrameTitle(window_t *parent, const char *txt_title)
        : window_frame_t(parent, parent->GetRect())
        , line(this, rect_line)
        , title(this, rect_title, is_multiline::no, is_closed_on_click_t::no, _(txt_title)) {
        line.SetBackColor(COLOR_WHITE);
        title.set_font(Font::big);
        static_cast<window_frame_t *>(parent)->CaptureNormalWindow(*this);
    }

protected:
    BasicWindow line;
    window_text_t title;
};

class FrameTitleRadio : public FrameTitle {

public:
    FrameTitleRadio(window_t *parent, Phase phase, const char *txt_title)
        : FrameTitle(parent, txt_title)
        , radio(this, WizardDefaults::RectRadioButton(0), phase) {
        CaptureNormalWindow(radio);
    }

protected:
    RadioButtonFSM radio;
};

class FrameTitleDescRadio : public FrameTitleRadio {

public:
    FrameTitleDescRadio(window_t *parent, Phase phase, const char *title, const char *desc)
        : FrameTitleRadio(parent, phase, title)
        , phase(phase)
        , desc_ptr(desc)
        , desc(this, rect_desc, is_multiline::yes) {}

    void update(fsm::PhaseData data) {
        if (phase == PhaseManualBeltTuning::show_tension) {
            const auto tensions = fsm::deserialize_data<belt_tensions>(data);
            desc.SetText(_(desc_ptr).formatted(params, tensions.get_upper(), manual_belt_tuning::freq_result_min, manual_belt_tuning::freq_result_max, tensions.get_lower(), manual_belt_tuning::freq_result_min, manual_belt_tuning::freq_result_max));
        } else {
            desc.SetText(_(desc_ptr));
        }
    }

protected:
    Phase phase;
    const char *desc_ptr;
    window_text_t desc;
    StringViewUtf8Parameters<30> params;
};

class FrameFinishJoe : public FrameTitleRadio {
public:
    FrameFinishJoe(window_t *parent, Phase phase, const char *title, const char *desc)
        : FrameTitleRadio(parent, phase, title)
        , desc(this, rect_text_joe, is_multiline::yes, is_closed_on_click_t::no, _(desc))
        , joe(this, rect_joe, &img::pepa_42x64) {
        joe.SetAlignment(Align_t::Center());
    }

protected:
    window_text_t desc;
    window_icon_t joe;
};

class FrameTitleDescRadioQR : public FrameTitleRadio {

public:
    FrameTitleDescRadioQR(window_t *parent, Phase phase, const char *title, const char *desc, const char *qr_link)
        : FrameTitleRadio(parent, phase, title)
        , phase(phase)
        , desc_ptr(desc)
        , qr(this, rect_qr, Align_t::Center(), qr_link)
        , scan_me(this, rect_scan_me, is_multiline::no, is_closed_on_click_t::no, _("Scan me"))
        , details(this, rect_details, is_multiline::no, is_closed_on_click_t::no, _("More details at"))
        , link(this, rect_link, is_multiline::no, is_closed_on_click_t::no, string_view_utf8::MakeRAM(qr_link))
        , desc(this, rect_desc_qr, is_multiline::yes) {
        qr.SetAlignment(Align_t::RightTop());
        details.set_font(Font::small);
        link.set_font(Font::small);
        scan_me.set_font(Font::small);
        scan_me.SetAlignment(Align_t::Center());
    }

    void update(fsm::PhaseData data) {
        if (phase == PhaseManualBeltTuning::adjust_tensioners) {

            auto revs = fsm::deserialize_data<screw_revs>(data);

            if (revs.turn_eights == 0) {
                desc.SetText(_(txt_desc_no_turn));
                return;
            }

            StringBuilder sb1(buffer_tighten_loosen);
            if (revs.turn_eights > 0) {
                sb1.append_string_view(_(txt_tighten));
            } else {
                sb1.append_string_view(_(txt_loosen));
            }

            StringBuilder sb2(buffer_eights);
            auto revs_abs = abs(revs.turn_eights);
            if (revs_abs >= 8) {
                const auto turns = revs_abs / 8;
                revs_abs %= 8;
                sb2.append_printf("%d & ", turns);
            }
            sb2.append_printf("%d/8", revs_abs);

            desc.SetText(_(desc_ptr).formatted(params, buffer_tighten_loosen.data(), buffer_eights.data()));

        } else {
            desc.SetText(_(desc_ptr));
        }
    }

protected:
    Phase phase;
    const char *desc_ptr;
    QRStaticStringWindow qr;
    window_text_t scan_me;
    window_text_t details;
    window_text_t link;
    window_text_t desc;
    std::array<char, 40> buffer_tighten_loosen = {};
    std::array<char, 40> buffer_eights = {};
    StringViewUtf8Parameters<120> params;
};

class FrameAdjustKnob : public FrameTitle {

public:
    FrameAdjustKnob(window_t *parent, Phase phase, const char *title, const char *desc)
        : FrameTitle(parent, title)
        , phase(phase)
        , desc(this, rect_desc_knob, is_multiline::yes, is_closed_on_click_t::no, _(desc))
        , numb(this, rect_numb, 0, "%0.1f Hz")
        , knob(this, rect_knob, &img::turn_knob_81x55)
        , plus(this, rect_plus, is_multiline::no, is_closed_on_click_t::no, string_view_utf8::MakeRAM("+"))
        , minus(this, rect_minus, is_multiline::no, is_closed_on_click_t::no, string_view_utf8::MakeRAM("-")) {
        plus.SetAlignment(Align_t::Center());
        plus.set_font(Font::big);
        plus.SetTextColor(COLOR_ORANGE);
        minus.SetAlignment(Align_t::Center());
        minus.set_font(Font::big);
        minus.SetTextColor(COLOR_ORANGE);
        numb.SetAlignment(Align_t::Center());
    }

    void update(fsm::PhaseData data) {
        const auto tensions = fsm::deserialize_data<belt_tensions>(data);
        if (phase == PhaseManualBeltTuning::measure_upper_belt) {
            numb.SetValue(tensions.get_upper());
        } else if (phase == PhaseManualBeltTuning::measure_lower_belt) {
            numb.SetValue(tensions.get_lower());
        }
    }

    virtual void windowEvent([[maybe_unused]] window_t *sender, GUI_event_t event, [[maybe_unused]] void *param) override {
        switch (event) {
        case GUI_event_t::CLICK:
            marlin_client::FSM_response(phase, Response::Done);
            break;
        default:
            break;
        }
    }

private:
    Phase phase;
    window_text_t desc;
    window_numb_t numb;
    window_icon_t knob;
    window_text_t plus;
    window_text_t minus;
};

} // namespace frames

namespace {

using FrameIntro = WithConstructorArgs<frames::FrameTitleDescRadioQR, Phase::intro, txt_title_begin, txt_desc_begin, link_begin_calib>;
using FrameCheckXGantry = WithConstructorArgs<frames::FrameTitleDescRadioQR, Phase::check_x_gantry, txt_title_gantry, txt_desc_gantry, link_belt_calib_gantry>;
using FrameWait = WithConstructorArgs<frames::FrameWait>;
using FrameIntroMeasure = WithConstructorArgs<frames::FrameTitleDescRadioQR, Phase::intro_measure, txt_title_measure, txt_desc_measure, link_begin_calib>;
using FrameMeasureUpBelt = WithConstructorArgs<frames::FrameAdjustKnob, Phase::measure_upper_belt, txt_title_up_belt_freq, txt_desc_up_belt_freq>;
using FrameMeasureLoBelt = WithConstructorArgs<frames::FrameAdjustKnob, Phase::measure_lower_belt, txt_title_lo_belt_freq, txt_desc_lo_belt_freq>;
using FrameShowTensions = WithConstructorArgs<frames::FrameTitleDescRadio, Phase::show_tension, txt_title_freq_report, txt_desc_freq_report>;
using FrameAlignmentIssue = WithConstructorArgs<frames::FrameTitleDescRadioQR, Phase::alignment_issue, txt_title_alignment_issue, txt_desc_alignment_issue, link_belt_calib_gantry>;
using FrameAdjustTensioners = WithConstructorArgs<frames::FrameTitleDescRadioQR, Phase::adjust_tensioners, txt_title_turn_screw, txt_desc_turn_screw, link_tensioning>;
using FrameFinished = WithConstructorArgs<frames::FrameFinishJoe, Phase::finished, txt_title_finished, txt_desc_finished>;

using Frames = FrameDefinitionList<ScreenBeltTuning::FrameStorage,
    FrameDefinition<Phase::intro, FrameIntro>,
    FrameDefinition<Phase::check_x_gantry, FrameCheckXGantry>,
    FrameDefinition<Phase::homing_wait, FrameWait>,
    FrameDefinition<Phase::intro_measure, FrameIntroMeasure>,
    FrameDefinition<Phase::measure_upper_belt, FrameMeasureUpBelt>,
    FrameDefinition<Phase::measure_lower_belt, FrameMeasureLoBelt>,
    FrameDefinition<Phase::show_tension, FrameShowTensions>,
    FrameDefinition<Phase::alignment_issue, FrameAlignmentIssue>,
    FrameDefinition<Phase::adjust_tensioners, FrameAdjustTensioners>,
    FrameDefinition<Phase::finished, FrameFinished>>;

} // namespace

ScreenBeltTuning::ScreenBeltTuning()
    : ScreenFSM("BELT TUNING", GuiDefaults::RectScreenNoHeader) {
    header.SetIcon(&img::selftest_16x16);
    CaptureNormalWindow(inner_frame);
    create_frame();
}

ScreenBeltTuning::~ScreenBeltTuning() {
    destroy_frame();
}

void ScreenBeltTuning::create_frame() {
    Frames::create_frame(frame_storage, get_phase(), &inner_frame);
}

void ScreenBeltTuning::destroy_frame() {
    Frames::destroy_frame(frame_storage, get_phase());
}

void ScreenBeltTuning::update_frame() {
    Frames::update_frame(frame_storage, get_phase(), fsm_base_data.GetData());
}
