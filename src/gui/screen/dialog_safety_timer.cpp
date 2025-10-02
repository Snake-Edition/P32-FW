#include "dialog_safety_timer.hpp"

#include <fsm/safety_timer_phases.hpp>
#include <gui/standard_frame/frame_prompt.hpp>
#include <gui/standard_frame/frame_progress_prompt.hpp>
#include <gui/standard_frame/frame_extensions/with_footer.hpp>

using Phase = PhaseSafetyTimer;

class FrameResuming : public WithFooter<FrameProgressPrompt, { footer::Item::nozzle, footer::Item::bed }> {

public:
    FrameResuming(window_frame_t *parent, Phase phase)
        : WithFooter(parent, phase, N_("Resuming temperatures"), N_("Resuming to print temperatures after safety timer timeout.")) {
    }

    void update(fsm::PhaseData data) {
        progress_bar.set_progress_percent(fsm::deserialize_data<float>(data));
    }
};

using Frames = FrameDefinitionList<DialogSafetyTimer::FrameStorage,
    FrameDefinition<Phase::resuming, FrameResuming>,
    FrameDefinition<Phase::abort_confirm, FramePrompt, N_("Abort print?"), N_("Do you really want to abort the print?")> //
    >;

DialogSafetyTimer::DialogSafetyTimer(fsm::BaseData data)
    : DialogFSM(data) {
    create_frame();
}

DialogSafetyTimer::~DialogSafetyTimer() {
    destroy_frame();
}

void DialogSafetyTimer::create_frame() {
    const auto phase = GetEnumFromPhaseIndex<Phase>(fsm_base_data.GetPhase());
    Frames::create_frame(frame_storage, phase, &inner_frame, phase);
}
void DialogSafetyTimer::destroy_frame() {
    const auto phase = GetEnumFromPhaseIndex<Phase>(fsm_base_data.GetPhase());
    Frames::destroy_frame(frame_storage, phase);
}
void DialogSafetyTimer::update_frame() {
    const auto phase = GetEnumFromPhaseIndex<Phase>(fsm_base_data.GetPhase());
    Frames::update_frame(frame_storage, phase, fsm_base_data.GetData());
}
