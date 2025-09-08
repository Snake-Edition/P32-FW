#include "screen_nozzle_cleaning.hpp"
#include "find_error.hpp"
#include "meta_utils.hpp"
#include "standard_frame/frame_qr_prompt.hpp"
#include <fsm/nozzle_cleaning_phases.hpp>
#include <standard_frame/frame_prompt.hpp>
#include <standard_frame/frame_prompt.hpp>
#include <img_resources.hpp>

using Phase = PhaseNozzleCleaning;
using Screen = ScreenNozzleCleaning;

namespace {
struct NozzleCleaningErr {
    static constexpr const char *str = find_error(ErrCode::CONNECT_NOZZLE_CLEANING_FAILED).err_text;
    inline operator string_view_utf8() const {
        return _(str);
    }
};
constexpr NozzleCleaningErr txt_desc_cleaning_failed;

constexpr auto txt_title_cleaning_nozzle = N_("Cleaning the nozzle");

constexpr auto txt_desc_recommend_purge = N_("We recommend purging the filament and removing it manually from the nozzle. Be carefull, the nozzle is hot.");
constexpr auto txt_desc_remove_filament = N_("Remove the purged filament and ensure the nozzle is clean and ready.");

constexpr auto warning_suffix = "nozzle-cleaning-failed"_tstr;

using Frames = FrameDefinitionList<Screen::FrameStorage,
    FrameDefinition<Phase::cleaning_failed, WithConstructorArgs<FrameQRPrompt, Phase::cleaning_failed, txt_desc_cleaning_failed, warning_suffix>>,
    FrameDefinition<Phase::recommend_purge, WithConstructorArgs<FramePrompt, Phase::recommend_purge, txt_title_cleaning_nozzle, txt_desc_recommend_purge>>,
    FrameDefinition<Phase::remove_filament, WithConstructorArgs<FramePrompt, Phase::remove_filament, txt_title_cleaning_nozzle, txt_desc_remove_filament>>>;
} // namespace

ScreenNozzleCleaning::ScreenNozzleCleaning()
    : ScreenFSM("WARNING", GuiDefaults::RectScreenNoHeader) {
    header.SetIcon(&img::warning_16x16);
    CaptureNormalWindow(inner_frame);
    create_frame();
}

ScreenNozzleCleaning::~ScreenNozzleCleaning() {
    destroy_frame();
}

void ScreenNozzleCleaning::create_frame() {
    Frames::create_frame(frame_storage, get_phase(), &inner_frame);
}

void ScreenNozzleCleaning::destroy_frame() {
    Frames::destroy_frame(frame_storage, get_phase());
}

void ScreenNozzleCleaning::update_frame() {
    Frames::update_frame(frame_storage, get_phase(), fsm_base_data.GetData());
}
