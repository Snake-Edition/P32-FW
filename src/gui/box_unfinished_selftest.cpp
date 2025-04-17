#include "box_unfinished_selftest.hpp"
#include <selftest_result_type.hpp>
#include "printers.h"
#include <option/has_selftest.h>
#include <option/has_sheet_profiles.h>
#include <config_store/store_instance.hpp>
#include <option/has_switched_fan_test.h>
#include <option/has_toolchanger.h>
#include <find_error.hpp>

#if HAS_TOOLCHANGER()
    #include <module/prusa/toolchanger.h>
#endif /* HAS_TOOLCHANGER() */

#if HAS_SHEET_PROFILES()
    #include <common/SteelSheets.hpp>
#endif

bool selftest_warning_selftest_finished() {
#if DEVELOPER_MODE() || !HAS_SELFTEST() || PRINTER_IS_PRUSA_iX()
    return true;
#endif

    const SelftestResult sr = config_store().selftest_result.get();

    const auto all_passed = [](std::same_as<TestResult> auto... results) -> bool {
        static_assert(sizeof...(results) > 0, "Pass at least one result");

        return ((results == TestResult_Passed) && ...); // all passed
    };

    if (!all_passed(sr.xaxis, sr.yaxis, sr.zaxis, sr.bed)) {
        return false;
    }

#if HAS_SWITCHED_FAN_TEST()
    HOTEND_LOOP() {
    #if HAS_TOOLCHANGER()
        if (!prusa_toolchanger.is_tool_enabled(e)) {
            continue;
        }
    #endif /* HAS_TOOLCHANGER() */

        if (sr.tools[e].fansSwitched != TestResult_Passed) {
            return false;
        }
    }
#endif /* HAS_SWITCHHED_FAN_TEST() */

#if (PRINTER_IS_PRUSA_XL())
    if (!all_passed(config_store().selftest_result_phase_stepping.get())) {
        return false;
    }
    for (int8_t e = 0; e < HOTENDS; e++) {
        if (!prusa_toolchanger.is_tool_enabled(e)) {
            continue;
        }
        if (!all_passed(sr.tools[e].printFan, sr.tools[e].heatBreakFan,
                sr.tools[e].nozzle, sr.tools[e].fsensor, sr.tools[e].loadcell)) {
            return false;
        }
        if (prusa_toolchanger.is_toolchanger_enabled()) {
            if (!all_passed(sr.tools[e].dockoffset, sr.tools[e].tooloffset)) {
                return false;
            }
        }
    }

    return true;
#elif (PRINTER_IS_PRUSA_MK4())
    if (sr.gears == TestResult_Failed) { // skipped/unknown gears are also OK
        return false;
    }

    HOTEND_LOOP()
    if (!all_passed(sr.tools[e].printFan, sr.tools[e].heatBreakFan, sr.tools[e].nozzle, sr.tools[e].fsensor, sr.tools[e].loadcell)) {
        return false;
    }

    return true;
#elif (PRINTER_IS_PRUSA_COREONE())
    if (!all_passed(sr.xaxis, sr.yaxis, sr.zaxis, sr.bed)) {
        return false;
    }

    if (sr.gears == TestResult_Failed) { // skipped/unknown gears are also OK
        return false;
    }

    HOTEND_LOOP()
    if (!all_passed(sr.tools[e].printFan, sr.tools[e].heatBreakFan, sr.tools[e].nozzle, sr.tools[e].fsensor, sr.tools[e].loadcell, sr.tools[e].fansSwitched)) {
        return false;
    }

    return true;
#elif PRINTER_IS_PRUSA_iX()

    HOTEND_LOOP()
    if (!all_passed(sr.tools[e].printFan, sr.tools[e].heatBreakFan, sr.tools[e].nozzle, sr.tools[e].fsensor, sr.tools[e].loadcell)) {
        return false;
    }

    return true;
#elif PRINTER_IS_PRUSA_MK3_5() || PRINTER_IS_PRUSA_MINI()

    if (!SteelSheets::IsSheetCalibrated(config_store().active_sheet.get())) {
        return false;
    }

    HOTEND_LOOP()
    if (!all_passed(sr.tools[e].printFan, sr.tools[e].heatBreakFan, sr.tools[e].nozzle)) {
        return false;
    }

    return true;

#else
    assert(false && "Not yet implemented");
    return false;
#endif
}

void warn_unfinished_selftest_msgbox() {
    if (!selftest_warning_selftest_finished()) {
        const auto &error = find_error(ErrCode::CONNECT_UNFINISHED_SELFTEST);
        MsgBoxWarning(_(error.err_text), Responses_Ok);
    }
}
