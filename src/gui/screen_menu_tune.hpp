/**
 * @file screen_menu_tune.hpp
 */
#pragma once

#include "screen_menu.hpp"
#include "MItem_hardware.hpp"
#include "MItem_print.hpp"
#include "MItem_tools.hpp"
#include "MItem_crash.hpp"
#include "MItem_menus.hpp"
#include "MItem_mmu.hpp"
#include <device/board.h>
#include "config_features.h"
#include <option/has_emergency_stop.h>
#include <option/has_chamber_api.h>
#include <option/has_loadcell.h>
#include <option/has_phase_stepping_toggle.h>
#include <option/has_toolchanger.h>
#include <option/developer_mode.h>
#include <option/has_mmu2.h>
#include <option/has_xbuddy_extension.h>
#include <option/has_chamber_filtration_api.h>
#include <device/board.h>

#if XL_ENCLOSURE_SUPPORT()
    #include "MItem_enclosure.hpp"
#endif
#if HAS_CHAMBER_API()
    #include <gui/menu_item/specific/menu_items_chamber.hpp>
#endif
#if HAS_XBUDDY_EXTENSION()
    #include <gui/menu_item/specific/menu_items_xbuddy_extension.hpp>
#endif

/*****************************************************************************/
// parent alias
using ScreenMenuTune__ = ScreenMenu<EFooter::On, MI_RETURN,
#if !HAS_LOADCELL()
    MI_LIVE_ADJUST_Z, // position without loadcell
#endif
    MI_M600,
#if ENABLED(CANCEL_OBJECTS)
    MI_CO_CANCEL_OBJECT,
#endif
    MI_SPEED,
    MI_NOZZLE<0>,
#if HAS_TOOLCHANGER()
    MI_NOZZLE<1>, MI_NOZZLE<2>, MI_NOZZLE<3>, MI_NOZZLE<4>,
#endif
    MI_HEATBED,
#if HAS_CHAMBER_API()
    MI_CHAMBER_TARGET_TEMP,
#endif
    MI_PRINTFAN,
#if HAS_XBUDDY_EXTENSION()
    MI_XBUDDY_EXTENSION_COOLING_FANS,
    MI_XBUDDY_EXTENSION_COOLING_FANS_CONTROL_MAX,
    MI_XBE_FILTRATION_FAN,
#endif
#if HAS_CHAMBER_FILTRATION_API()
    MI_CHAMBER_FILTRATION,
#endif
#if HAS_LOADCELL()
    MI_LIVE_ADJUST_Z, // position with loadcell
#endif
    MI_FLOWFACT<0>,
#if HAS_TOOLCHANGER()
    MI_FLOWFACT<1>, MI_FLOWFACT<2>, MI_FLOWFACT<3>, MI_FLOWFACT<4>,
#endif /*HAS_TOOLCHANGER()*/
#if HAS_FILAMENT_SENSORS_MENU()
    MI_FILAMENT_SENSORS,
#else
    MI_FILAMENT_SENSOR,
#endif
#if HAS_LOADCELL()
    MI_STUCK_FILAMENT_DETECTION,
#endif
#if XL_ENCLOSURE_SUPPORT()
    MI_ENCLOSURE_ENABLE,
    MI_ENCLOSURE,
#endif
    MI_STEALTH_MODE,
    MI_SOUND_MODE,
#if HAS_ST7789_DISPLAY()
    // We could potentionally have MINI display without buzzer.
    // So we only allow sound control for ST7789
    MI_SOUND_VOLUME,
#endif
    MI_INPUT_SHAPER,
#if HAS_PHASE_STEPPING_TOGGLE()
    MI_PHASE_STEPPING_TOGGLE,
#endif
    MI_FAN_CHECK,
    MI_GCODE_VERIFY,
#if HAS_EMERGENCY_STOP()
    MI_EMERGENCY_STOP_ENABLE,
#endif
#if HAS_MMU2()
    MI_MMU_CUTTER,
#endif
#if ENABLED(CRASH_RECOVERY)
    MI_CRASH_DETECTION,
    MI_CRASH_SENSITIVITY_XY,
#endif // ENABLED(CRASH_RECOVERY)
    MI_USER_INTERFACE, MI_NETWORK,
#if (!PRINTER_IS_PRUSA_MINI()) || defined(_DEBUG) // Save space in MINI release
    MI_HARDWARE_TUNE,
#endif /*(!PRINTER_IS_PRUSA_MINI()) || defined(_DEBUG)*/
    MI_TIMEZONE, MI_TIMEZONE_MIN, MI_TIMEZONE_SUMMER, MI_INFO,
#if ENABLED(POWER_PANIC)
    MI_TRIGGER_POWER_PANIC,
#endif

    MI_SNAKE_TUNE_SETTINGS,

/* MI_FOOTER_SETTINGS,*/ // currently experimental, but we want it in future
#if DEVELOPER_MODE()
    MI_ERROR_TEST,
#endif
    MI_MESSAGES>;

class ScreenMenuTune : public ScreenMenuTune__ {
public:
    constexpr static const char *label = N_("TUNE");
    ScreenMenuTune();

protected:
    virtual void windowEvent(window_t *sender, GUI_event_t event, void *param) override;
};
