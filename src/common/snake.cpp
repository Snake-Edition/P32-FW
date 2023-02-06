#include "snake.h"
#include "extruder_enum.h"
#include "hwio.h"
#include "eeprom.h"
#include "crc32.h"
#include "app.h"
#include "config_buddy_2209_02.h"
#include "marlin_client.h"
#include "marlin_server.h"
#include "st25dv64k.h"
#include "cmsis_os.h"
#include "../../lib/Marlin/Marlin/src/module/planner.h"

void llama_apply_fan_settings() {
    switch (eeprom_get_ui8(EEVAR_LLAMA_HOTEND_FAN_SPEED)) {
    case eHOTEND_FAN_SPEED::HOTEND_FAN_SPEED_100:
        hwio_fan_control_set_hotend_fan_speed_percent(100);
        break;
    case eHOTEND_FAN_SPEED::HOTEND_FAN_SPEED_90:
        hwio_fan_control_set_hotend_fan_speed_percent(90);
        break;
    case eHOTEND_FAN_SPEED::HOTEND_FAN_SPEED_80:
        hwio_fan_control_set_hotend_fan_speed_percent(80);
        break;
    case eHOTEND_FAN_SPEED::HOTEND_FAN_SPEED_70:
        hwio_fan_control_set_hotend_fan_speed_percent(70);
        break;
    case eHOTEND_FAN_SPEED::HOTEND_FAN_SPEED_60:
        hwio_fan_control_set_hotend_fan_speed_percent(60);
        break;
    case eHOTEND_FAN_SPEED::HOTEND_FAN_SPEED_50:
        hwio_fan_control_set_hotend_fan_speed_percent(50);
        break;
    case eHOTEND_FAN_SPEED::HOTEND_FAN_SPEED_DEFAULT:
    default:
        hwio_fan_control_set_hotend_fan_speed_percent(38);
        break;
    }
}

void llama_apply_skew_settings() {
    double xy, xz, yz;
    if (eeprom_get_bool(EEVAR_LLAMA_SKEW_ENABLED)) {
        xy = eeprom_get_flt(EEVAR_LLAMA_SKEW_XY);
        xz = eeprom_get_flt(EEVAR_LLAMA_SKEW_XZ);
        yz = eeprom_get_flt(EEVAR_LLAMA_SKEW_YZ);
    } else {
        xy = xz = yz = 0.f;
    }
    if (marlin_is_client_thread()) {
        // client thread - set variables remotely
        marlin_gcode_printf("M852 I%f J%f K%f", xy, xz, yz);
    } else {
        // server thread - set variables directly
        marlin_server_vars()->skew_xy = xy;
        marlin_server_handle_var_change(MARLIN_VAR_SKEW_XY);
        marlin_server_vars()->skew_xz = xz;
        marlin_server_handle_var_change(MARLIN_VAR_SKEW_XZ);
        marlin_server_vars()->skew_yz = yz;
        marlin_server_handle_var_change(MARLIN_VAR_SKEW_YZ);
    }
}
