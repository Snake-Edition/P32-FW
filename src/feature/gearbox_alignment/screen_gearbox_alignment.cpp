/// @file
#include "screen_gearbox_alignment.hpp"

#include "frame_calibration_common.hpp"
#include "i18n.h"
#include "img_resources.hpp"
#include "window_icon.hpp"
#include <gui/selftest_frame.hpp>
#include <guiconfig/wizard_config.hpp>

static ScreenGearboxAlignment *instance = nullptr;

static const char *text_header = N_("GEARBOX ALIGNMENT");

static constexpr const img::Resource &right_icon = img::transmission_loose_187x175;

static constexpr size_t content_top_y = WizardDefaults::row_1 + WizardDefaults::progress_row_h;
static constexpr size_t text_icon_space = 14;
static constexpr size_t text_left_width = WizardDefaults::X_space - right_icon.w - text_icon_space;

class FrameText {
protected:
    FrameText(window_t *parent, string_view_utf8 txt)
        : text(parent, Rect16(WizardDefaults::col_0, content_top_y, WizardDefaults::RectSelftestFrame.Width() - WizardDefaults::MarginRight - WizardDefaults::MarginLeft, right_icon.h), is_multiline::yes) {
        text.SetText(txt);
    }

private:
    window_text_t text;
};

class FrameTextAndImage {
protected:
    FrameTextAndImage(window_t *parent, string_view_utf8 txt, const img::Resource *res)
        : text(parent, Rect16(WizardDefaults::col_0, content_top_y, text_left_width, right_icon.h), is_multiline::yes)
        , icon(parent, &right_icon, point_i16_t(WizardDefaults::RectSelftestFrame.Width() - WizardDefaults::MarginRight - right_icon.w, content_top_y)) {
        text.SetText(txt);
        icon.SetRes(res);
    }

private:
    window_text_t text;
    window_icon_t icon;
};

class FrameIntro final : public FrameText {
public:
    FrameIntro(window_t *parent)
        : FrameText {
            parent,
            _("The gearbox alignment is only necessary for user-assembled or serviced gearboxes. In all other cases, you can skip this step."),
        } {}
};

class FrameFilamentLoadedAskUnload final : public FrameText {
public:
    FrameFilamentLoadedAskUnload(window_t *parent)
        : FrameText {
            parent,
            _("We need to start without the filament in the extruder. Please unload it."),
        } {}
};

class FrameFilamentUnknownAskUnload final : public FrameText {
public:
    FrameFilamentUnknownAskUnload(window_t *parent)
        : FrameText {
            parent,
            _("Before you proceed, make sure filament is unloaded from the Nextruder."),
        } {}
};

class FrameLoosenScrews final : public FrameTextAndImage {
public:
    FrameLoosenScrews(window_t *parent)
        : FrameTextAndImage {
            parent,
            _("Rotate each screw counter-clockwise by 1.5 turns. The screw heads should be flush with the cover. Unlock and open the idler."),
            &img::transmission_loose_187x175,
        } {}
};

class FrameAlignment final : public FrameTextAndImage {
public:
    FrameAlignment(window_t *parent)
        : FrameTextAndImage {
            parent,
            _("Gearbox alignment in progress, please wait (approx. 20 seconds)"),
            &img::transmission_gears_187x175,
        } {}
};

class FrameTightenScrews final : public FrameTextAndImage {
public:
    FrameTightenScrews(window_t *parent)
        : FrameTextAndImage {
            parent,
            _("Tighten the M3 screws firmly in the correct order, they should be slightly below the surface. Do not over-tighten."),
            &img::transmission_tight_187x175,
        } {}
};

class FrameDone final : public FrameTextAndImage {
public:
    FrameDone(window_t *parent)
        : FrameTextAndImage {
            parent,
            _("Close the idler door and secure it with the swivel. The calibration is done!"),
            &img::transmission_close_187x175,
        } {}
};

using Frames = FrameDefinitionList<ScreenGearboxAlignment::FrameStorage,
    FrameDefinition<PhaseGearboxAlignment::intro, FrameIntro>,
    FrameDefinition<PhaseGearboxAlignment::filament_loaded_ask_unload, FrameFilamentLoadedAskUnload>,
    FrameDefinition<PhaseGearboxAlignment::filament_unknown_ask_unload, FrameFilamentUnknownAskUnload>,
    FrameDefinition<PhaseGearboxAlignment::loosen_screws, FrameLoosenScrews>,
    FrameDefinition<PhaseGearboxAlignment::alignment, FrameAlignment>,
    FrameDefinition<PhaseGearboxAlignment::tighten_screws, FrameTightenScrews>,
    FrameDefinition<PhaseGearboxAlignment::done, FrameDone>>;

static PhaseGearboxAlignment get_phase(const fsm::BaseData &fsm_base_data) {
    return GetEnumFromPhaseIndex<PhaseGearboxAlignment>(fsm_base_data.GetPhase());
}

ScreenGearboxAlignment::ScreenGearboxAlignment()
    : ScreenFSM { text_header, rect_screen }
    , radio(this, rect_radio, PhaseGearboxAlignment::finish) {
    CaptureNormalWindow(radio);
    create_frame();
    instance = this;
}

ScreenGearboxAlignment::~ScreenGearboxAlignment() {
    instance = nullptr;
    destroy_frame();
}

ScreenGearboxAlignment *ScreenGearboxAlignment::GetInstance() {
    return instance;
}

void ScreenGearboxAlignment::create_frame() {
    Frames::create_frame(frame_storage, get_phase(fsm_base_data), this);
    radio.set_fsm_and_phase(get_phase(fsm_base_data));
    radio.Invalidate(); // TODO investigate why this sometimes doesn't get invalidated
}

void ScreenGearboxAlignment::destroy_frame() {
    Frames::destroy_frame(frame_storage, get_phase(fsm_base_data));
}

void ScreenGearboxAlignment::update_frame() {
    Frames::update_frame(frame_storage, get_phase(fsm_base_data), fsm_base_data.GetData());
}
