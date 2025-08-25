#pragma once

#include <screen_fsm.hpp>
#include <marlin_server_types/fsm/manual_belt_tuning_phases.hpp>

class ScreenManualBeltTuning final : public ScreenFSM {

public:
    ScreenManualBeltTuning();
    ~ScreenManualBeltTuning();

protected:
    inline PhaseManualBeltTuning get_phase() const {
        return GetEnumFromPhaseIndex<PhaseManualBeltTuning>(fsm_base_data.GetPhase());
    }

protected:
    void create_frame() override;
    void destroy_frame() override;
    void update_frame() override;
};
