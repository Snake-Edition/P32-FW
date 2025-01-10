#include "gui.hpp"
#include "screen_menu.hpp"
#include "WindowMenuItems.hpp"
#include "MItem_menus.hpp"
#include "MItem_tools.hpp"
#include "MItem_experimental_tools.hpp"

using ScreenMenuSnakeSettings__ = ScreenMenu<EFooter::On, MI_RETURN
#if PRINTER_IS_PRUSA_MINI()
    ,
    MI_BRIGHTNESS //, MI_DISPLAY_REINIT_TIMEOUT
#endif
    /*,MI_MBL_VALUES*/,
    MI_SKEW_XY, MI_SKEW_XZ, MI_SKEW_YZ
#if PRINTER_IS_PRUSA_MINI()
// ,MI_X_AXIS_LEN, MI_Y_AXIS_LEN, MI_E_LOAD_LENGTH
#endif
    ,
    MI_PID_SETTINGS
    // ,MI_CURRENT_X, MI_CURRENT_Y, MI_CURRENT_Z, MI_CURRENT_E, MI_RESET_CURRENTS
    // ,MI_X_MAX_FEEDRATE, MI_Y_MAX_FEEDRATE
    // ,MI_X_SENSITIVITY, MI_X_SENSITIVITY_RESET, MI_Y_SENSITIVITY, MI_Y_SENSITIVITY_RESET
    // ,MI_X_STEALTH, MI_Y_STEALTH, MI_Z_STEALTH, MI_E_STEALTH
    // ,MI_COLD_MODE
    >;

class ScreenMenuSnakeSettings : public ScreenMenuSnakeSettings__ {
public:
    ScreenMenuSnakeSettings();
    virtual void windowEvent(window_t *sender, GUI_event_t ev, void *param) override;
};

/*********************************************************************************/
// Submenu for PID settings
using ScreenMenuPIDSettings__ = ScreenMenu<EFooter::On, MI_RETURN,
    MI_NOZZLE_CALIBRATION_TEMP, MI_CALIBRATE_NOZZLE_PID, MI_BED_CALIBRATION_TEMP, MI_CALIBRATE_BED_PID,
    MI_PID_NOZZLE_P, MI_PID_NOZZLE_I, MI_PID_NOZZLE_D,
    MI_PID_BED_P, MI_PID_BED_I, MI_PID_BED_D>;

class ScreenMenuPIDSettings : public ScreenMenuPIDSettings__ {
public:
    ScreenMenuPIDSettings();
    virtual void windowEvent(window_t *sender, GUI_event_t ev, void *param) override;
};

/*********************************************************************************/

using ScreenMenuSnakeTuneSettings__ = ScreenMenu<EFooter::On, MI_RETURN
#if PRINTER_IS_PRUSA_MINI()
    ,
    MI_BRIGHTNESS //, MI_DISPLAY_REINIT_TIMEOUT
#endif
                  //,MI_MBL_VALUES
    ,
    MI_SKEW_XY, MI_SKEW_XZ, MI_SKEW_YZ, MI_CURRENT_X, MI_CURRENT_Y, MI_CURRENT_Z, MI_CURRENT_E, MI_RESET_CURRENTS
    //,MI_X_MAX_FEEDRATE, MI_Y_MAX_FEEDRATE
    //,MI_X_STEALTH, MI_Y_STEALTH, MI_Z_STEALTH, MI_E_STEALTH
    //,MI_COLD_MODE
    ,
    MI_SNAKE>;

class ScreenMenuSnakeTuneSettings : public ScreenMenuSnakeTuneSettings__ {
public:
    ScreenMenuSnakeTuneSettings();
    virtual void windowEvent(window_t *sender, GUI_event_t ev, void *param) override;
};

/*********************************************************************************/

using ScreenMenuControlTuneSettings__ = ScreenMenu<EFooter::On, MI_RETURN,
    MI_MOVE_AXIS
    //,MI_X_HOME, MI_Y_HOME
    >;

class ScreenMenuControlTuneSettings : public ScreenMenuControlTuneSettings__ {
public:
    ScreenMenuControlTuneSettings();
    virtual void windowEvent(window_t *sender, GUI_event_t ev, void *param) override;
};

/*********************************************************************************/

using ScreenMenuSettingsTuneSettings__ = ScreenMenu<EFooter::On, MI_RETURN,
    MI_INPUT_SHAPER, MI_USER_INTERFACE, MI_LANG_AND_TIME, MI_NETWORK>;

class ScreenMenuSettingsTuneSettings : public ScreenMenuSettingsTuneSettings__ {
public:
    ScreenMenuSettingsTuneSettings();
    virtual void windowEvent(window_t *sender, GUI_event_t ev, void *param) override;
};
