#pragma once
#include "i18n.h"
#include <utility_extensions.hpp>
#include <common/selftest/include_XL/printer_selftest.hpp>
#include <option/has_precise_homing_corexy.h>

namespace SelftestSnake {
enum class Tool {
    Tool1 = 0,
    Tool2 = 1,
    Tool3 = 2,
    Tool4 = 3,
    Tool5 = 4,
    _count,
    _all_tools = _count,
    _last = _count - 1,
    _first = Tool1,
};

constexpr Tool operator-(Tool tool, int i) {
    assert(std::to_underlying(tool) - i >= std::to_underlying(Tool::_first));
    return static_cast<Tool>(std::to_underlying(tool) - i);
}

constexpr Tool operator+(Tool tool, int i) {
    assert(std::to_underlying(tool) + i <= std::to_underlying(Tool::_last));
    return static_cast<Tool>(std::to_underlying(tool) + i);
}

// Order matters, snake and will be run in the same order, as well as menu items (with indices) will be
enum class Action {
    Fans,
    YCheck,
    XCheck,
#if HAS_PRECISE_HOMING_COREXY()
    PreciseHoming,
#endif
    ZAlign, // also known as z_calib
    DockCalibration,
    Loadcell,
    ZCheck,
    Heaters,
    NozzleHeaters,
    Gears,
    FilamentSensorCalibration,
    ToolOffsetsCalibration,
    BedHeaters,
    PhaseSteppingCalibration,
    _count,
    _last = _count - 1,
    _first = Fans,
};

template <Action action>
concept SubmenuActionC = action == Action::DockCalibration || action == Action::Loadcell || action == Action::FilamentSensorCalibration || action == Action::Gears;

constexpr bool has_submenu(Action action) {
    switch (action) {
    case Action::DockCalibration:
    case Action::Loadcell:
    case Action::FilamentSensorCalibration:
    case Action::Gears:
        return true;
    default:
        return false;
    }
}

constexpr bool is_multitool_only_action(Action action) {
    return action == Action::DockCalibration || action == Action::ToolOffsetsCalibration || action == Action::NozzleHeaters || action == Action::BedHeaters;
}

constexpr bool requires_toolchanger(Action action) {
    return action == Action::DockCalibration || action == Action::ToolOffsetsCalibration;
}

constexpr bool is_singletool_only_action(Action action) {
    return action == Action::Heaters;
}

consteval auto get_submenu_label(Tool tool, Action action) -> const char * {
    struct ToolText {
        Tool tool;
        Action action;
        const char *label;
    };
    const ToolText tooltexts[] {
        { Tool::Tool1, Action::DockCalibration, N_("Dock 1 Calibration") },
        { Tool::Tool2, Action::DockCalibration, N_("Dock 2 Calibration") },
        { Tool::Tool3, Action::DockCalibration, N_("Dock 3 Calibration") },
        { Tool::Tool4, Action::DockCalibration, N_("Dock 4 Calibration") },
        { Tool::Tool5, Action::DockCalibration, N_("Dock 5 Calibration") },
        { Tool::Tool1, Action::Loadcell, N_("Tool 1 Loadcell Test") },
        { Tool::Tool2, Action::Loadcell, N_("Tool 2 Loadcell Test") },
        { Tool::Tool3, Action::Loadcell, N_("Tool 3 Loadcell Test") },
        { Tool::Tool4, Action::Loadcell, N_("Tool 4 Loadcell Test") },
        { Tool::Tool5, Action::Loadcell, N_("Tool 5 Loadcell Test") },
        { Tool::Tool1, Action::FilamentSensorCalibration, N_("Tool 1 Filament Sensor Calibration") },
        { Tool::Tool2, Action::FilamentSensorCalibration, N_("Tool 2 Filament Sensor Calibration") },
        { Tool::Tool3, Action::FilamentSensorCalibration, N_("Tool 3 Filament Sensor Calibration") },
        { Tool::Tool4, Action::FilamentSensorCalibration, N_("Tool 4 Filament Sensor Calibration") },
        { Tool::Tool5, Action::FilamentSensorCalibration, N_("Tool 5 Filament Sensor Calibration") },
        { Tool::Tool1, Action::Gears, N_("Tool 1 Gearbox alignment") },
        { Tool::Tool2, Action::Gears, N_("Tool 2 Gearbox alignment") },
        { Tool::Tool3, Action::Gears, N_("Tool 3 Gearbox alignment") },
        { Tool::Tool4, Action::Gears, N_("Tool 4 Gearbox alignment") },
        { Tool::Tool5, Action::Gears, N_("Tool 5 Gearbox alignment") },

    };

    if (auto it = std::ranges::find_if(tooltexts, [&](const auto &elem) {
            return elem.tool == tool && elem.action == action;
        });
        it != std::end(tooltexts)) {
        return it->label;
    } else {
        consteval_assert_false("Unable to find a label for this combination");
        return "";
    }
}

struct MenuItemText {
    Action action;
    const char *label;
};

// could have been done with an array of texts directly, but there would be an order dependancy
inline constexpr MenuItemText blank_item_texts[] {
    { Action::Fans, N_("%d Fan Test") },
        { Action::ZAlign, N_("%d Z Alignment Calibration") },
        { Action::YCheck, N_("%d Y Axis Test") },
        { Action::XCheck, N_("%d X Axis Test") },
#if HAS_PRECISE_HOMING_COREXY()
        { Action::PreciseHoming, N_("%d Homing Calibration") },
#endif
        { Action::DockCalibration, N_("%d Dock Position Calibration") },
        { Action::Loadcell, N_("%d Loadcell Test") },
        { Action::ToolOffsetsCalibration, N_("%d Tool Offset Calibration") },
        { Action::ZCheck, N_("%d Z Axis Test") },
        { Action::Heaters, N_("%d Heater Test") },
        { Action::FilamentSensorCalibration, N_("%d Filament Sensor Calibration") },
        { Action::BedHeaters, N_("%d Bed Heater Test") },
        { Action::NozzleHeaters, N_("%d Nozzle Heaters Test") },
        { Action::PhaseSteppingCalibration, N_("%d Phase Stepping Calibration") },
        { Action::Gears, N_("%d Gearbox Alignment") },
};

TestResult get_test_result(Action action, Tool tool);
ToolMask get_tool_mask(Tool tool);
uint64_t get_test_mask(Action action);

/**
 * @brief Question user to choose configuration before doing the test.
 */
void ask_config(Action action);

Tool get_last_enabled_tool();

/**
 * @brief Get the next enabled tool.
 * @param tool current tool
 * @return next enabled tool
 */
Tool get_next_tool(Tool tool);
} // namespace SelftestSnake
