#include <selftest_result_evaluation.hpp>
#include <selftest_result_type.hpp>
#include <option/has_switched_fan_test.h>
#include <option/has_gearbox_alignment.h>
#include <option/has_selftest.h>
#include <option/has_phase_stepping_selftest.h>
#include <option/filament_sensor.h>
#include <option/has_loadcell.h>

#include <option/has_toolchanger.h>
#if HAS_TOOLCHANGER()
    #include <module/prusa/toolchanger.h>
#endif /* HAS_TOOLCHANGER() */

#include <option/has_sheet_profiles.h>
#if HAS_SHEET_PROFILES()
    #include <common/SteelSheets.hpp>
#endif /* HAS_SHEET_PROFILES() */

#include <config_store/store_instance.hpp>

bool is_selftest_successfully_completed() {
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

#if HAS_SHEET_PROFILES()
    if (!SteelSheets::IsSheetCalibrated(config_store().active_sheet.get())) {
        return false;
    }
#endif /* HAS_SHEET_PROFILES() */

    HOTEND_LOOP() {
#if HAS_TOOLCHANGER()
        if (!prusa_toolchanger.is_tool_enabled(e)) {
            continue;
        }

        if (!all_passed(sr.tools[e].dockoffset, sr.tools[e].tooloffset)) {
            return false;
        }
#endif /* HAS_TOOLCHANGER() */

#if HAS_SWITCHED_FAN_TEST()
        if (sr.tools[e].fansSwitched != TestResult_Passed) {
            return false;
        }
#endif /* HAS_SWITCHED_FAN_TEST() */

#if HAS_GEARBOX_ALIGNMENT()
        if (sr.tools[e].gears == TestResult_Failed) {
            return false;
        }
#endif /* HAS_GEARBOX_ALIGNMENT */

#if FILAMENT_SENSOR_IS_ADC()
        if (!all_passed(sr.tools[e].fsensor)) {
            return false;
        }
#endif /* FILAMENT_SENSOR_ADC() */

#if HAS_LOADCELL()
        if (!all_passed(sr.tools[e].loadcell)) {
            return false;
        }
#endif /* HAS_LOADCELL() */

        if (!all_passed(sr.tools[e].printFan, sr.tools[e].heatBreakFan, sr.tools[e].nozzle)) {
            return false;
        }
    }

#if HAS_PHASE_STEPPING_SELFTEST()
    if (!all_passed(config_store().selftest_result_phase_stepping.get())) {
        return false;
    }
#endif /* HAS_PHASE_STEPPING_SELFTEST() */

    return true;
}
