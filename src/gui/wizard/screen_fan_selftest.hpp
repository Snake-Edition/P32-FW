#pragma once

#include "screen_fsm.hpp"
#include "radio_button_fsm.hpp"

class ScreenFanSelftest : public ScreenFSM {
public:
    ScreenFanSelftest();
    ~ScreenFanSelftest();

protected:
    virtual void create_frame() override;
    virtual void destroy_frame() override;
    virtual void update_frame() override;

    PhasesFansSelftest get_phase() const {
        return GetEnumFromPhaseIndex<PhasesFansSelftest>(fsm_base_data.GetPhase());
    }
};
