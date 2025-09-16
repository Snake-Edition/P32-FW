/// \file

#pragma once

#include <screen_fsm.hpp>
#include <marlin_server_types/fsm/nozzle_cleaning_phases.hpp>

class ScreenNozzleCleaningFailed final : public ScreenFSM {

public:
    ScreenNozzleCleaningFailed();
    ~ScreenNozzleCleaningFailed();

protected:
    inline PhaseNozzleCleaningFailed get_phase() const {
        return GetEnumFromPhaseIndex<PhaseNozzleCleaningFailed>(fsm_base_data.GetPhase());
    }

protected:
    void create_frame() override;
    void destroy_frame() override;
    void update_frame() override;
};
