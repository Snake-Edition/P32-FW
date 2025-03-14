/**
 * @file screen_menu_move.hpp
 */
#pragma once

#include <meta_utils.hpp>
#include "screen_menu.hpp"
#include "WindowItemFormatableLabel.hpp"
#include "MItem_filament.hpp"

class I_MI_AXIS : public WiSpin {

public:
    I_MI_AXIS(size_t index);
    ~I_MI_AXIS();

public:
    /// Plans the move to the target location as one single move (uninterrupable) – used when closing the dialog
    void finish_move();

protected:
    // enqueue next moves according to value of spinners;
    void Loop() override;

protected:
    const size_t axis_index;

    static xyz_float_t last_queued_pos;
    static xyz_float_t target_position;
    static bool did_final_move;
};

class MI_AXIS_E : public I_MI_AXIS {

public:
    MI_AXIS_E()
        : I_MI_AXIS(E_AXIS) {}

    void OnClick() override;
    void Loop() override;

protected:
    float last_queued_position {};
};

class DUMMY_AXIS_E : public WI_FORMATABLE_LABEL_t<int> {

public:
    DUMMY_AXIS_E();

public:
    static bool IsTargetTempOk();

    // TODO call automatically in men loop
    void Update();

protected:
    void click(IWindowMenu &window_menu) override;
};

using MI_AXIS_X = WithConstructorArgs<I_MI_AXIS, X_AXIS>;
using MI_AXIS_Y = WithConstructorArgs<I_MI_AXIS, Y_AXIS>;
using MI_AXIS_Z = WithConstructorArgs<I_MI_AXIS, Z_AXIS>;

using ScreenMenuMove__ = ScreenMenu<EFooter::On, MI_RETURN, MI_AXIS_X, MI_AXIS_Y, MI_AXIS_Z, MI_AXIS_E, DUMMY_AXIS_E, MI_COOLDOWN>;

class ScreenMenuMove : public ScreenMenuMove__ {
    float prev_accel;

    void checkNozzleTemp();

public:
    constexpr static const char *label = N_("MOVE AXIS");
    static constexpr int temp_ok = 170; ///< Allow moving extruder if temperature is above this value [degC]
    static bool IsTempOk();

    ScreenMenuMove();
    ~ScreenMenuMove();

protected:
    void windowEvent(window_t *sender, GUI_event_t event, void *param) override;
};
