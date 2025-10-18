#include "box_unfinished_selftest.hpp"
#include <selftest_result_type.hpp>
#include "printers.h"
#include <option/has_selftest.h>
#include <option/has_sheet_profiles.h>
#include <config_store/store_instance.hpp>
#include <option/has_switched_fan_test.h>
#include <option/has_toolchanger.h>
#include <find_error.hpp>
#include <marlin_server.hpp>
#include <client_response.hpp>
#include <window_msgbox.hpp>

// Responses for unfinished selftest dialog
inline constexpr PhaseResponses Responses_IgnoreCalibrate = { Response::Ignore, Response::Calibrate, Response::_none, Response::_none };

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

void mark_selftest_as_passed_and_unblock() {
#if HAS_SELFTEST()
    // Set all selftest results to Passed and clear run_selftest
    auto &store = config_store();
    {
        auto transaction = store.get_backend().transaction_guard();
        SelftestResult sr = store.selftest_result.get();
        sr.xaxis = TestResult_Passed;
        sr.yaxis = TestResult_Passed;
        sr.zaxis = TestResult_Passed;
        sr.bed = TestResult_Passed;
        sr.eth = TestResultNet_Up;
        sr.wifi = TestResultNet_Up;
        sr.zalign = TestResult_Passed;
        sr.gears = TestResult_Passed;
        for (uint8_t i = 0; i < config_store_ns::max_tool_count; ++i) {
            sr.tools[i].printFan = TestResult_Passed;
            sr.tools[i].heatBreakFan = TestResult_Passed;
            sr.tools[i].fansSwitched = TestResult_Passed;
            sr.tools[i].nozzle = TestResult_Passed;
            sr.tools[i].fsensor = TestResult_Passed;
            sr.tools[i].loadcell = TestResult_Passed;
            sr.tools[i].sideFsensor = TestResult_Passed;
            sr.tools[i].dockoffset = TestResult_Passed;
            sr.tools[i].tooloffset = TestResult_Passed;
        }
        store.selftest_result.set(sr);

#if HAS_PHASE_STEPPING()
        store.selftest_result_phase_stepping.set(TestResult_Passed);
#endif

#if HAS_INPUT_SHAPER_CALIBRATION()
        store.selftest_result_input_shaper_calibration.set(TestResult_Passed);
#endif

        // Do not show selftest on next boot
        store.run_selftest.set(false);
    }

#if HAS_SHEET_PROFILES()
    // Ensure at least one sheet is considered calibrated
    // If active sheet is uncalibrated, set its z offset to 0
    const auto active = store.active_sheet.get();
    auto sheet = store.get_sheet(active);
    if (!(sheet.z_offset >= SteelSheets::zOffsetMin && sheet.z_offset <= SteelSheets::zOffsetMax)) {
        sheet.z_offset = 0.0f;
        store.set_sheet(active, sheet);
    }
#endif
#endif // HAS_SELFTEST()
}

void warn_unfinished_selftest_msgbox() {
    if (!selftest_warning_selftest_finished()) {
        const auto &error = find_error(ErrCode::CONNECT_UNFINISHED_SELFTEST);
        Response resp = MsgBoxWarning(_(error.err_text), Responses_IgnoreCalibrate);
        
        switch (resp) {
        case Response::Ignore:
            // User chose to ignore the warning; mark tests as passed to mirror legacy behavior
            mark_selftest_as_passed_and_unblock();
            break;
        case Response::Calibrate:
            // User chose to run calibration
            marlin_server::request_calibrations_screen();
            break;
        default:
            break;
        }
    }
}
