#pragma once

#include "screen_toolhead_settings_common.hpp"

namespace screen_toolhead_settings {

class MI_DOCK_X : public MI_TOOLHEAD_SPECIFIC_SPIN {
public:
    MI_DOCK_X(Toolhead toolhead = default_toolhead);
    float read_value_impl(ToolheadIndex ix) final;
    void store_value_impl(ToolheadIndex ix, float set) final;
};

class MI_DOCK_Y : public MI_TOOLHEAD_SPECIFIC_SPIN {
public:
    MI_DOCK_Y(Toolhead toolhead = default_toolhead);
    float read_value_impl(ToolheadIndex ix) final;
    void store_value_impl(ToolheadIndex ix, float set) final;
};

#if HAS_SELFTEST()
class MI_DOCK_CALIBRATE : public MI_TOOLHEAD_SPECIFIC_BASE<IWindowMenuItem> {
public:
    MI_DOCK_CALIBRATE(Toolhead toolhead = default_toolhead);
    void click(IWindowMenu &);
    void update() final {}
};
#endif

using ScreenToolheadDetailDock_ = ScreenMenu<EFooter::Off,
    MI_RETURN,
    MI_DOCK_X,
    MI_DOCK_Y //
#if HAS_SELFTEST()
    ,
    MI_DOCK_CALIBRATE
#endif
    >;

class ScreenToolheadDetailDock : public ScreenToolheadDetailDock_ {
public:
    ScreenToolheadDetailDock(Toolhead toolhead = default_toolhead);

private:
    const Toolhead toolhead;
};

} // namespace screen_toolhead_settings
