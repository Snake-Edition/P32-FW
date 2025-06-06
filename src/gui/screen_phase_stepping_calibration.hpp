#pragma once

#include "screen_fsm.hpp"
#include "radio_button_fsm.hpp"
#include <option/has_phase_stepping_calibration.h>

static_assert(HAS_PHASE_STEPPING_CALIBRATION());

class ScreenPhaseSteppingCalibration : public ScreenFSM {
private:
    RadioButtonFSM radio;

public:
    ScreenPhaseSteppingCalibration();
    ~ScreenPhaseSteppingCalibration();

    static constexpr Rect16 get_inner_frame_rect() {
        return GuiDefaults::RectScreenBody - GuiDefaults::GetButtonRect(GuiDefaults::RectScreenBody).Height();
    }

protected:
    virtual void create_frame() override;
    virtual void destroy_frame() override;
    virtual void update_frame() override;

    PhasesPhaseStepping get_phase() const {
        return GetEnumFromPhaseIndex<PhasesPhaseStepping>(fsm_base_data.GetPhase());
    }
};
