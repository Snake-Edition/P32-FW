#include "screen_nozzle_cleaning_failed.hpp"
#include <standard_frame/frame_extensions/with_footer.hpp>
#include <footer_line.hpp>
#include <standard_frame/frame_progress_prompt.hpp>
#include <find_error.hpp>
#include <standard_frame/frame_qr_prompt.hpp>
#include <fsm/nozzle_cleaning_failed_phases.hpp>
#include <standard_frame/frame_prompt.hpp>
#include <standard_frame/frame_prompt.hpp>
#include <img_resources.hpp>

using Phase = PhaseNozzleCleaningFailed;

namespace {

constexpr auto txt_title_cleaning_nozzle = N_("Cleaning the nozzle");
constexpr auto txt_warning = N_("Warning");

constexpr auto txt_desc_cleaning_failed = N_("\nNozzle cleaning failed.");

#if HAS_NOZZLE_CLEANING_FAILED_PURGING()
constexpr auto txt_desc_recommend_purge = N_("Would you like to purge the filament?\n\nIt will then retract to prevent oozing. Be careful, the nozzle is hot!");
constexpr auto txt_desc_wait_temp = N_("Waiting for nozzle temperature...");
constexpr auto txt_desc_purge = N_("Purging the filament.\n\nPlease wait until the purge is complete.");
constexpr auto txt_desc_autoretract = N_("The filament has been purged.\n\nThe nozzle will now retract the filament to prevent oozing.");
constexpr auto txt_desc_remove_filament = N_("Remove the purged filament and ensure the nozzle is clean and ready.\n\nBe careful, the nozzle is hot!");
#endif

#if HAS_AUTO_RETRACT()
constexpr auto txt_desc_offer_auto_retract = N_("The auto-retract feature is disabled, which might have caused the failure.\n\nDo you want to enable auto-retract?");
#endif

constexpr auto txt_desc_warn_abort = N_("Are you sure you want to abort the print?\n\nThe current print will be cancelled and you will need to start over.");

constexpr auto warning_suffix = "nozzle-cleaning-failed"_tstr;

class FrameNozzleCleaningProgress : public FrameProgressPrompt {
public:
    FrameNozzleCleaningProgress(window_frame_t *parent, Phase fsm_phase, const string_view_utf8 &txt_title, const string_view_utf8 &txt_info)
        : FrameProgressPrompt(parent, fsm_phase, txt_title, txt_info, Align_t::CenterTop()) {
    }

    void update(const fsm::PhaseData &data_) {
        const auto data = fsm::deserialize_data<NozzleCleaningFailedProgressData>(data_);
        progress_bar.SetProgressPercent(static_cast<float>(data.progress_0_255) / 255.0f * 100.0f);
    }
};

constexpr FooterLine::IdArray nozzle_cleaning_footer_items = { footer::Item::nozzle, footer::Item::bed };

using Frames = FrameDefinitionList<ScreenNozzleCleaningFailed::FrameStorage,
    FrameDefinition<Phase::cleaning_failed, WithConstructorArgs<FrameQRPrompt, Phase::cleaning_failed, txt_desc_cleaning_failed, warning_suffix>>,
#if HAS_NOZZLE_CLEANING_FAILED_PURGING()
    FrameDefinition<Phase::recommend_purge, WithConstructorArgs<WithFooter<FramePrompt, nozzle_cleaning_footer_items>, Phase::recommend_purge, txt_title_cleaning_nozzle, txt_desc_recommend_purge>>,
    FrameDefinition<Phase::wait_temp, WithConstructorArgs<WithFooter<FrameNozzleCleaningProgress, nozzle_cleaning_footer_items>, Phase::wait_temp, txt_title_cleaning_nozzle, txt_desc_wait_temp>>,
    FrameDefinition<Phase::purge, WithConstructorArgs<WithFooter<FrameNozzleCleaningProgress, nozzle_cleaning_footer_items>, Phase::purge, txt_title_cleaning_nozzle, txt_desc_purge>>,
    FrameDefinition<Phase::autoretract, WithConstructorArgs<WithFooter<FrameNozzleCleaningProgress, nozzle_cleaning_footer_items>, Phase::autoretract, txt_title_cleaning_nozzle, txt_desc_autoretract>>,
    FrameDefinition<Phase::remove_filament, WithConstructorArgs<WithFooter<FramePrompt, nozzle_cleaning_footer_items>, Phase::remove_filament, txt_title_cleaning_nozzle, txt_desc_remove_filament>>,
#endif
#if HAS_AUTO_RETRACT()
    FrameDefinition<Phase::offer_auto_retract_enable, WithConstructorArgs<FramePrompt, Phase::offer_auto_retract_enable, txt_desc_cleaning_failed, txt_desc_offer_auto_retract>>,
#endif
    FrameDefinition<Phase::warn_abort, WithConstructorArgs<WithFooter<FramePrompt, nozzle_cleaning_footer_items>, Phase::warn_abort, txt_warning, txt_desc_warn_abort>>>;

} // namespace

ScreenNozzleCleaningFailed::ScreenNozzleCleaningFailed()
    : ScreenFSM(N_("WARNING"), GuiDefaults::RectScreenNoHeader) {
    header.SetIcon(&img::warning_16x16);
    CaptureNormalWindow(inner_frame);
    create_frame();
}

ScreenNozzleCleaningFailed::~ScreenNozzleCleaningFailed() {
    destroy_frame();
}

void ScreenNozzleCleaningFailed::create_frame() {
    Frames::create_frame(frame_storage, get_phase(), &inner_frame);
}

void ScreenNozzleCleaningFailed::destroy_frame() {
    Frames::destroy_frame(frame_storage, get_phase());
}

void ScreenNozzleCleaningFailed::update_frame() {
    Frames::update_frame(frame_storage, get_phase(), fsm_base_data.GetData());
}
