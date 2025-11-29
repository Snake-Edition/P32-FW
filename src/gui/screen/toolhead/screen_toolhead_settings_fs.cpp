#include "screen_toolhead_settings_fs.hpp"

using namespace screen_toolhead_settings;

static constexpr NumericInputConfig fs_ref_spin_config = {
    .max_value = 99999,
    .special_value = -1,
    .special_value_str = N_("-"),
};

// * MI_FS_REF_NINS
MI_FS_REF_NINS::MI_FS_REF_NINS(Toolhead toolhead)
    : MI_TOOLHEAD_SPECIFIC_SPIN(toolhead, 0, fs_ref_spin_config, string_view_utf8::MakeCPUFLASH("FS NINS Ref")) {
    update();
}

float MI_FS_REF_NINS::read_value_impl(ToolheadIndex ix) {
    return config_store().get_extruder_fs_ref_nins_value(ix);
}

void MI_FS_REF_NINS::store_value_impl(ToolheadIndex ix, float set) {
    config_store().set_extruder_fs_ref_nins_value(ix, set);
}

// * MI_FS_REF_INS
MI_FS_REF_INS::MI_FS_REF_INS(Toolhead toolhead)
    : MI_TOOLHEAD_SPECIFIC_SPIN(toolhead, 0, fs_ref_spin_config, string_view_utf8::MakeCPUFLASH("FS INS Ref")) {
    update();
}

float MI_FS_REF_INS::read_value_impl(ToolheadIndex ix) {
    return config_store().get_extruder_fs_ref_ins_value(ix);
}

void MI_FS_REF_INS::store_value_impl(ToolheadIndex ix, float set) {
    config_store().set_extruder_fs_ref_ins_value(ix, set);
}

#if HAS_ADC_SIDE_FSENSOR()
// * MI_SIDE_FS_REF_NINS
MI_SIDE_FS_REF_NINS::MI_SIDE_FS_REF_NINS(Toolhead toolhead)
    : MI_TOOLHEAD_SPECIFIC_SPIN(toolhead, 0, fs_ref_spin_config, string_view_utf8::MakeCPUFLASH("Side FS NINS Ref")) {
    update();
}

float MI_SIDE_FS_REF_NINS::read_value_impl(ToolheadIndex ix) {
    return config_store().get_side_fs_ref_nins_value(ix);
}

void MI_SIDE_FS_REF_NINS::store_value_impl(ToolheadIndex ix, float set) {
    config_store().set_side_fs_ref_nins_value(ix, set);
}

// * MI_SIDE_FS_REF_INS
MI_SIDE_FS_REF_INS::MI_SIDE_FS_REF_INS(Toolhead toolhead)
    : MI_TOOLHEAD_SPECIFIC_SPIN(toolhead, 0, fs_ref_spin_config, string_view_utf8::MakeCPUFLASH("Side FS INS Ref")) {
    update();
}

float MI_SIDE_FS_REF_INS::read_value_impl(ToolheadIndex ix) {
    return config_store().get_side_fs_ref_ins_value(ix);
}

void MI_SIDE_FS_REF_INS::store_value_impl(ToolheadIndex ix, float set) {
    config_store().set_side_fs_ref_ins_value(ix, set);
}
#endif

// * ScreenToolheadDetailFS
ScreenToolheadDetailFS::ScreenToolheadDetailFS(Toolhead toolhead)
    : ScreenMenu(_("FILAMENT SENSORS"))
    , toolhead(toolhead) //
{
    menu_set_toolhead(container, toolhead);
}
