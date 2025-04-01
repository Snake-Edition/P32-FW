#pragma once

#include <variant>
#include <str_utils.hpp>

struct GCodeFilename {
    ConstexprString name;
    ConstexprString fallback { nullptr };
};

/*
 * The user is responsible for ensuring the G-code string has the correct format
 * specifier (e.g., "%f" or "%d") if a parameter is provided. For example:
 *   GCodeLiteral("M600 T%d P", 3.0f);
 *
 * When no parameter is necessary, omit it or pass std::nullopt:
 *   GCodeLiteral("M17");
 *
 * It is up to the caller to perform the actual string formatting if needed.
 * NaN is representing empty parameter.
 */
struct GCodeLiteral {
    ConstexprString gcode;
    float parameter = std::numeric_limits<float>::quiet_NaN();
};

struct GCodeMacroButton {
    uint8_t button;
};

using InjectQueueRecord = std::variant<GCodeFilename, GCodeMacroButton, GCodeLiteral>;
