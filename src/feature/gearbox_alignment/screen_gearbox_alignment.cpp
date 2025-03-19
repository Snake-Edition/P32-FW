/// @file
#include "screen_gearbox_alignment.hpp"

#include <frame_calibration_common.hpp>
#include <i18n.h>
#include <img_resources.hpp>
#include <guiconfig/wizard_config.hpp>

static ScreenGearboxAlignment *instance = nullptr;

static const char *text_header = N_("GEARBOX ALIGNMENT");
static constexpr size_t content_top_y = WizardDefaults::row_1 + WizardDefaults::progress_row_h;

class FrameIntro final : public FrameText {
public:
    FrameIntro(window_t *parent)
        : FrameText {
            parent,
            _("The gearbox alignment is only necessary for user-assembled or serviced gearboxes. In all other cases, you can skip this step."),
            content_top_y,
        } {}
};

class FrameFilamentLoadedAskUnload final : public FrameText {
public:
    FrameFilamentLoadedAskUnload(window_t *parent)
        : FrameText {
            parent,
            _("We need to start without the filament in the extruder. Please unload it."),
            content_top_y,
        } {}
};

class FrameFilamentUnknownAskUnload final : public FrameText {
public:
    FrameFilamentUnknownAskUnload(window_t *parent)
        : FrameText {
            parent,
            _("Before you proceed, make sure filament is unloaded from the Nextruder."),
            content_top_y,
        } {}
};

class FrameLoosenScrews final : public FrameTextWithImage {
public:
    FrameLoosenScrews(window_t *parent)
        : FrameTextWithImage {
            parent,
            _("Rotate each screw counter-clockwise by 1.5 turns. The screw heads should be flush with the cover. Unlock and open the idler."),
            content_top_y,
            &img::transmission_loose_187x175,
            187,
        } {}
};

class FrameAlignment final : public FrameTextWithImage {
public:
    FrameAlignment(window_t *parent)
        : FrameTextWithImage {
            parent,
            _("Gearbox alignment in progress, please wait (approx. 20 seconds)"),
            content_top_y,
            &img::transmission_gears_187x175,
            187,
        } {}
};

class FrameTightenScrews final : public FrameTextWithImage {
public:
    FrameTightenScrews(window_t *parent)
        : FrameTextWithImage {
            parent,
            _("Tighten the M3 screws firmly in the correct order, they should be slightly below the surface. Do not over-tighten."),
            content_top_y,
            &img::transmission_tight_187x175,
            187,
        } {}
};

class FrameDone final : public FrameTextWithImage {
public:
    FrameDone(window_t *parent)
        : FrameTextWithImage {
            parent,
            _("Close the idler door and secure it with the swivel. The calibration is done!"),
            content_top_y,
            &img::transmission_close_187x175,
            187,
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
