#include "gui.hpp"
#include "screen_menu.hpp"
#include "WindowMenuItems.hpp"
#include "MItem_menus.hpp"
#include "MItem_tools.hpp"
#include "screen_menus.hpp"
#include "menu_spin_config.hpp"
#include "snake.h"
#include "extruder_enum.h"

using Screen = ScreenMenu<EFooter::On, MI_RETURN,
    MI_BRIGHTNESS, MI_HOTEND_FAN_SPEED, MI_SKEW_ENABLED, MI_SKEW_XY, MI_SKEW_XZ, MI_SKEW_YZ, MI_COLD_MODE, MI_NOZZLE_CALIBRATION_TEMP, MI_CALIBRATE_NOZZLE_PID, MI_BED_CALIBRATION_TEMP, MI_CALIBRATE_BED_PID>;

class ScreenMenuSnakeSettings : public Screen {
    constexpr static const char *label = N_("SNAKE SETTINGS");

public:
    ScreenMenuSnakeSettings()
        : Screen(_(label)) {
        // last_extruder_type = variant8_get_ui8(eeprom_snake_get_var(EEVAR_SNAKE_EXTRUDER_TYPE));
    }

    uint8_t last_extruder_type;

    virtual void windowEvent(EventLock /*has private ctor*/, window_t *sender, GUI_event_t ev, void *param) override {
        if (ev != GUI_event_t::CHILD_CLICK) {
            SuperWindowEvent(sender, ev, param);
            return;
        }
    }
};

ScreenFactory::UniquePtr GetScreenMenuSnakeSettings() {
    return ScreenFactory::Screen<ScreenMenuSnakeSettings>();
}

/*********************************************************************************/

using ScreenTune = ScreenMenu<EFooter::On, MI_RETURN,
    MI_HOTEND_FAN_SPEED, MI_SKEW_ENABLED, MI_SKEW_XY, MI_SKEW_XZ, MI_SKEW_YZ>;

class ScreenMenuSnakeTuneSettings : public ScreenTune {
    constexpr static const char *label = N_("SNAKE SETTINGS");

public:
    ScreenMenuSnakeTuneSettings()
        : ScreenTune(_(label)) {
    }

    uint8_t last_extruder_type;

    virtual void windowEvent(EventLock /*has private ctor*/, window_t *sender, GUI_event_t ev, void *param) override {
        if (ev != GUI_event_t::CHILD_CLICK) {
            SuperWindowEvent(sender, ev, param);
            return;
        }
    }
};

ScreenFactory::UniquePtr GetScreenMenuSnakeTuneSettings() {
    return ScreenFactory::Screen<ScreenMenuSnakeTuneSettings>();
}
