#include "selftest_snake_config.hpp"
#include <selftest_types.hpp>
#include <screen_menu_selftest_snake_result_parsing.hpp>
#include <config_store/store_instance.hpp>
#include <option/has_switched_fan_test.h>

#include <option/has_chamber_filtration_api.h>

#include <option/has_chamber_api.h>
#if HAS_CHAMBER_API()
    #include <feature/chamber/chamber.hpp>
#endif

#include <option/has_xbuddy_extension.h>
#if HAS_XBUDDY_EXTENSION()
    #include <feature/xbuddy_extension/xbuddy_extension.hpp>
#endif

using namespace buddy;

namespace SelftestSnake {
TestResult get_test_result(Action action, [[maybe_unused]] Tool tool) {

    SelftestResult sr = config_store().selftest_result.get();

    switch (action) {
    case Action::Fans: {
        TestResult res = merge_hotends_evaluations(
            [&](int8_t e) {
                return evaluate_results(sr.tools[e].evaluate_fans());
            });
#if HAS_CHAMBER_API()
        switch (chamber().backend()) {
    #if HAS_XBUDDY_EXTENSION()
        case Chamber::Backend::xbuddy_extension: {
            const auto chamber_results = config_store().xbe_fan_test_results.get();
            static_assert(HAS_CHAMBER_FILTRATION_API());
            if (buddy::xbuddy_extension().using_filtration_fan_instead_of_cooling_fans()) {
                res = evaluate_results(res, chamber_results.fans[2]);
            } else {
                res = evaluate_results(res, chamber_results.fans[0]);
                res = evaluate_results(res, chamber_results.fans[1]);
            }
            break;
        }
    #endif /* HAS_XBUDDY_EXTENSION() */
        case Chamber::Backend::none:
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
    case Action::Loadcell:
        if (tool == Tool::_all_tools) {
            return merge_hotends_evaluations(
                [&](int8_t e) {
                    return evaluate_results(sr.tools[e].loadcell);
                });
        } else {
            return evaluate_results(sr.tools[ftrstd::to_underlying(tool)].loadcell);
        }
    case Action::ZCheck:
        return evaluate_results(sr.zaxis);
    case Action::Heaters:
        return evaluate_results(sr.bed, merge_hotends_evaluations([&](int8_t e) {
            return evaluate_results(sr.tools[e].nozzle);
        }));
    case Action::Gears:
        return evaluate_results(sr.gears);
    case Action::DoorSensor:
        return evaluate_results(config_store().selftest_result_door_sensor.get());
    case Action::FilamentSensorCalibration:
        if (tool == Tool::_all_tools) {
            return merge_hotends_evaluations(
                [&](int8_t e) {
                    return evaluate_results(sr.tools[e].fsensor);
                });
        } else {
            return evaluate_results(sr.tools[ftrstd::to_underlying(tool)].fsensor);
        }
    case Action::_count:
        break;
    }
    return TestResult_Unknown;
}

ToolMask get_tool_mask([[maybe_unused]] Tool tool) {
    return ToolMask::AllTools;
}

uint64_t get_test_mask(Action action) {
    switch (action) {
    case Action::Fans:
    case Action::DoorSensor:
        bsod("This should be gcode");
    case Action::YCheck:
        return stmYAxis;
    case Action::XCheck:
        return stmXAxis;
    case Action::ZCheck:
        return stmZAxis;
    case Action::Heaters:
        return stmHeaters;
    case Action::FilamentSensorCalibration:
        return stmFSensor;
    case Action::Loadcell:
        return stmLoadcell;
    case Action::ZAlign:
        return stmZcalib;
    case Action::Gears:
        return stmGears;
    case Action::_count:
        break;
    }
    assert(false);
    return stmNone;
}

} // namespace SelftestSnake
