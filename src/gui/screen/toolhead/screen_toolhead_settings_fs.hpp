#pragma once

#include "screen_toolhead_settings_common.hpp"

#include <option/has_adc_side_fsensor.h>

namespace screen_toolhead_settings {

class MI_FS_REF_NINS : public MI_TOOLHEAD_SPECIFIC_SPIN {
public:
    MI_FS_REF_NINS(Toolhead toolhead = default_toolhead);
    float read_value_impl(ToolheadIndex ix) final;
    void store_value_impl(ToolheadIndex ix, float set) final;
};

class MI_FS_REF_INS : public MI_TOOLHEAD_SPECIFIC_SPIN {
public:
    MI_FS_REF_INS(Toolhead toolhead = default_toolhead);
    float read_value_impl(ToolheadIndex ix) final;
    void store_value_impl(ToolheadIndex ix, float set) final;
};

#if HAS_ADC_SIDE_FSENSOR()
class MI_SIDE_FS_REF_NINS : public MI_TOOLHEAD_SPECIFIC_SPIN {
public:
    MI_SIDE_FS_REF_NINS(Toolhead toolhead = default_toolhead);
    float read_value_impl(ToolheadIndex ix) final;
    void store_value_impl(ToolheadIndex ix, float set) final;
};

class MI_SIDE_FS_REF_INS : public MI_TOOLHEAD_SPECIFIC_SPIN {
public:
    MI_SIDE_FS_REF_INS(Toolhead toolhead = default_toolhead);
    float read_value_impl(ToolheadIndex ix) final;
    void store_value_impl(ToolheadIndex ix, float set) final;
};
#endif

using ScreenToolheadDetailFS_ = ScreenMenu<EFooter::Off,
    MI_RETURN,
    MI_FS_REF_NINS,
    MI_FS_REF_INS
#if HAS_ADC_SIDE_FSENSOR()
    ,
    MI_SIDE_FS_REF_NINS,
    MI_SIDE_FS_REF_INS
#endif
    >;

class ScreenToolheadDetailFS : public ScreenToolheadDetailFS_ {
public:
    ScreenToolheadDetailFS(Toolhead toolhead = default_toolhead);

private:
    const Toolhead toolhead;
};

} // namespace screen_toolhead_settings
