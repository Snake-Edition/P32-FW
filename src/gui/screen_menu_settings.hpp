/**
 * @file screen_menu_settings.hpp
 */
#pragma once

#include "screen_menu.hpp"
#include "WindowMenuItems.hpp"
#include "MItem_menus.hpp"
#include "MItem_tools.hpp"
#include "knob_event.hpp"
#include "MItem_crash.hpp"
#include "Configuration_adv.h"
#include <option/has_mmu2.h>
#include <option/developer_mode.h>
#include <option/has_xbuddy_extension.h>
#include <option/has_phase_stepping.h>

#if HAS_MMU2()
    #include "MItem_mmu.hpp"
#endif

#if HAS_XBUDDY_EXTENSION()
    #include <gui/menu_item/specific/menu_items_xbuddy_extension.hpp>
#endif

#if HAS_PHASE_STEPPING()
    #include "screen_menu_phase_stepping.hpp"
#endif

#include <option/has_chamber_filtration_api.h>
#if HAS_CHAMBER_FILTRATION_API()
    #include <gui/menu_item/specific/menu_items_chamber_filtration.hpp>
#endif

class MI_HELP_FW_UPDATE : public IWindowMenuItem {
    static constexpr const char *const label = N_("FW update");

public:
    MI_HELP_FW_UPDATE();

protected:
    virtual void click(IWindowMenu &window_menu) override;
};

/*****************************************************************************/

using ScreenMenuSettings__ = ScreenMenu<GuiDefaults::MenuFooter, MI_RETURN,
    MI_SNAKE_SETTINGS,
#if HAS_FILAMENT_SENSORS_MENU()
    MI_FILAMENT_SENSORS,
#else
    MI_FILAMENT_SENSOR,
#endif
#if HAS_LOADCELL()
    MI_STUCK_FILAMENT_DETECTION,
#endif
#if HAS_MMU2()
    MI_MMU_ENABLE,
    MI_MMU_BOOTLOADER_RESULT,
    MI_MMU_CUTTER,
#endif
#if HAS_XBUDDY_EXTENSION()
    MI_CAM_USB_PWR,
#endif
    MI_STEALTH_MODE,
    MI_FAN_CHECK,
    MI_GCODE_VERIFY,
    MI_DRYRUN,
#if HAS_CHAMBER_FILTRATION_API()
    MI_CHAMBER_FILTRATION,
#endif
#if ENABLED(CRASH_RECOVERY)
    MI_CRASH_DETECTION,
#endif // ENABLED(CRASH_RECOVERY)
#if HAS_TOOLCHANGER()
    MI_TOOLHEAD_SETTINGS,
#endif
    MI_INPUT_SHAPER,
#if HAS_PHASE_STEPPING()
    MI_PHASE_STEPPING_SCREEN,
#endif
#if DEVELOPER_MODE()
    MI_ERROR_TEST,
#endif
    MI_USER_INTERFACE, MI_LANG_AND_TIME, MI_NETWORK, MI_HARDWARE, MI_HELP_FW_UPDATE, MI_EXPERIMENTAL_SETTINGS,
    // MI_SYSTEM needs to be last to ensure we can safely hit factory reset even in presence of unknown languages
    MI_SYSTEM>;

class ScreenMenuSettings : public ScreenMenuSettings__ {
    gui::knob::screen_action_cb old_action;

public:
    ScreenMenuSettings();
    ~ScreenMenuSettings();
};
