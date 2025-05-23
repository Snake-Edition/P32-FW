#pragma once
#include <selftest_snake_config.hpp>
#include <inc/Conditionals_LCD.h>
#include <printers.h>

namespace SelftestSnake {

constexpr TestResult evaluate_results(std::same_as<TestResult> auto... results) {
    static_assert(sizeof...(results) > 0, "Pass at least one result");

    if (((results == TestResult_Passed) && ... && true)) { // all passed
        return TestResult_Passed;
    } else if (((results == TestResult_Failed) || ... || false)) { // any failed
        return TestResult_Failed;
    } else if (((results == TestResult_Skipped) || ... || false)) { // any skipped
        return TestResult_Skipped;
    } else { // only unknowns and passed (max n-1) are left
        return TestResult_Unknown;
    }
}

constexpr TestResult merge_hotends_evaluations(std::invocable<int8_t> auto evaluate_one) {
    TestResult res { TestResult_Passed };
    for (int8_t e = 0; e < HOTENDS; e++) {
#if HAS_TOOLCHANGER()
        if (!prusa_toolchanger.is_tool_enabled(e)) {
            continue;
        }
#endif
        res = evaluate_results(res, evaluate_one(e));
    }
    return res;
};

constexpr TestResult merge_hotends(Tool tool, stdext::inplace_function<TestResult(int8_t)> evaluate) {
    if (tool == Tool::_all_tools) {
        return merge_hotends_evaluations(evaluate);
    } else {
        return evaluate(std::to_underlying(tool));
    }
}

}; // namespace SelftestSnake

/**
 *  Check if all essential selftest passed
 */
bool is_selftest_successfully_completed();
