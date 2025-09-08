/// \file

#pragma once

#include <screen_fsm.hpp>
#include <marlin_server_types/fsm/nozzle_cleaning_phases.hpp>

class ScreenNozzleCleaning final : public ScreenFSM {

public:
    ScreenNozzleCleaning();
    ~ScreenNozzleCleaning();

protected:
    inline PhaseNozzleCleaning get_phase() const {
        return GetEnumFromPhaseIndex<PhaseNozzleCleaning>(fsm_base_data.GetPhase());
    }

protected:
    void create_frame() override;
    void destroy_frame() override;
    void update_frame() override;
};
