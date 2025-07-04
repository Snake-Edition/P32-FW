#include "selftest_snake_config.hpp"
#include <selftest_types.hpp>
#include <selftest_result_evaluation.hpp>
#include <config_store/store_instance.hpp>

#include <option/has_side_fsensor.h>
#include <option/has_chamber_api.h>
#if HAS_CHAMBER_API()
    #include <feature/chamber/chamber.hpp>
#endif
#include <option/xl_enclosure_support.h>
#include <option/has_toolchanger.h>
#if HAS_TOOLCHANGER()
    #include <module/prusa/toolchanger.h>
    #if HAS_SIDE_FSENSOR()
        #include <filament_sensors_handler_XL_remap.hpp>
    #endif /*HAS_SIDE_FSENSOR()*/
#endif /*HAS_TOOLCHANGER()*/

#include <option/has_precise_homing_corexy.h>

#if HAS_PRECISE_HOMING_COREXY()
    #include <module/prusa/homing_corexy.hpp>
#endif

namespace SelftestSnake {
TestResult get_test_result(Action action, Tool tool) {
    SelftestResult sr = config_store().selftest_result.get();

    switch (action) {
    case Action::Fans: {
        TestResult res = merge_hotends_evaluations(
            [&](int8_t e) {
                return evaluate_results(sr.tools[e].evaluate_fans());
            });
#if HAS_CHAMBER_API()
        switch (buddy::chamber().backend()) {
    #if XL_ENCLOSURE_SUPPORT()
        case buddy::Chamber::Backend::xl_enclosure:
            res = evaluate_results(res, config_store().xl_enclosure_fan_selftest_result.get());
            break;
    #endif /* XL_ENCLOSURE_SUPPORT() */
        case buddy::Chamber::Backend::none:
            break;
        }
#endif /* HAS_CHAMBER_API() */
        return res;
    }
    case Action::ZAlign:
        return evaluate_results(sr.zalign);
    case Action::YCheck:
        return evaluate_results(sr.yaxis);
    case Action::XCheck:
        return evaluate_results(sr.xaxis);
#if HAS_PRECISE_HOMING_COREXY()
    case Action::PreciseHoming:
        return corexy_home_is_calibrated() ? TestResult::TestResult_Passed : TestResult::TestResult_Unknown;
#endif
    case Action::DockCalibration:
        return merge_hotends(tool, [&](const int8_t e) {
            return evaluate_results(sr.tools[e].dockoffset);
        });
    case Action::Loadcell:
        return merge_hotends(tool, [&](const int8_t e) {
            return evaluate_results(sr.tools[e].loadcell);
        });
    case Action::ToolOffsetsCalibration:
        return merge_hotends_evaluations(
            [&](int8_t e) {
                return evaluate_results(sr.tools[e].tooloffset);
            });
    case Action::ZCheck:
        return evaluate_results(sr.zaxis);
    case Action::BedHeaters:
        return evaluate_results(sr.bed);
    case Action::NozzleHeaters:
        return merge_hotends_evaluations([&](int8_t e) {
            return evaluate_results(sr.tools[e].nozzle);
        });
    case Action::Heaters:
        return evaluate_results(sr.bed, merge_hotends_evaluations([&](int8_t e) {
            return evaluate_results(sr.tools[e].nozzle);
        }));
    case Action::FilamentSensorCalibration:
        return merge_hotends(tool, [&](const int8_t e) {
            return evaluate_results(sr.tools[e].fsensor);
        });
    case Action::PhaseSteppingCalibration:
        return evaluate_results(config_store().selftest_result_phase_stepping.get());
    case Action::Gears:
        return merge_hotends(tool, [&](const int8_t e) {
            return evaluate_results(sr.tools[e].gears);
        });
    case Action::_count:
        break;
    }
    return TestResult_Unknown;
}

ToolMask get_tool_mask(Tool tool) {
#if HAS_TOOLCHANGER()
    switch (tool) {
    case Tool::Tool1:
        return ToolMask::Tool0;
    case Tool::Tool2:
        return ToolMask::Tool1;
    case Tool::Tool3:
        return ToolMask::Tool2;
    case Tool::Tool4:
        return ToolMask::Tool3;
    case Tool::Tool5:
        return ToolMask::Tool4;
        break;
    default:
        assert(false);
        break;
    }
#endif
    return ToolMask::AllTools;
}

uint64_t get_test_mask(Action action) {
    switch (action) {
    case Action::YCheck:
        return stmYAxis;
    case Action::XCheck:
        return stmXAxis;
    case Action::ZCheck:
        return stmZAxis;
    case Action::Heaters:
        return stmHeaters;
    case Action::BedHeaters:
        return stmHeaters_bed;
    case Action::NozzleHeaters:
        return stmHeaters_noz;
    case Action::Gears:
#if HAS_PRECISE_HOMING_COREXY()
    case Action::PreciseHoming:
#endif
        bsod("This should be gcode");
    case Action::FilamentSensorCalibration:
        return stmFSensor;
    case Action::Loadcell:
        return stmLoadcell;
    case Action::ZAlign:
        return stmZcalib;
    case Action::DockCalibration:
        return stmDocks;
    case Action::ToolOffsetsCalibration:
        return stmToolOffsets;
    case Action::Fans:
    case Action::PhaseSteppingCalibration:
        bsod("get_test_mask");
        break;
    case Action::_count:
        break;
    }
    assert(false);
    return stmNone;
}

void ask_config(Action action) {
    switch (action) {
    case Action::FilamentSensorCalibration: {
#if HAS_TOOLCHANGER() && HAS_SIDE_FSENSOR()
        side_fsensor_remap::ask_to_remap(); // Ask user whether to remap filament sensors
#endif /*HAS_TOOLCHANGER()*/
    } break;

    default:
        break;
    }
}

Tool get_last_enabled_tool() {
#if HAS_TOOLCHANGER()
    for (int i = EXTRUDERS - 1; i >= 0; --i) {
        if (prusa_toolchanger.is_tool_enabled(i)) {
            return static_cast<Tool>(i);
        }
    }
#endif /*HAS_TOOLCHANGER()*/
    return Tool::Tool1;
}

Tool get_next_tool(Tool tool) {
#if HAS_TOOLCHANGER()
    assert(tool != get_last_enabled_tool() && "Unhandled edge case");
    do {
        tool = tool + 1;
    } while (!prusa_toolchanger.is_tool_enabled(std::to_underlying(tool)));
#endif /*HAS_TOOLCHANGER()*/
    return tool;
}

} // namespace SelftestSnake
