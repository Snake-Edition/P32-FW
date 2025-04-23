#include "gui.hpp"
#include "screen_menu.hpp"
#include "WindowMenuItems.hpp"
#include "MItem_menus.hpp"
#include "MItem_tools.hpp"

using ScreenMenuSnakeSettings__ = ScreenMenu<EFooter::On, MI_RETURN,
#if PRINTER_IS_PRUSA_MINI()
    MI_BRIGHTNESS,
#endif
    MI_MBL_VALUES, MI_SKEW_XY, MI_SKEW_XZ, MI_SKEW_YZ,
#if PRINTER_IS_PRUSA_MINI()
    MI_X_AXIS_LEN, MI_Y_AXIS_LEN, MI_E_LOAD_LENGTH,
#endif

    MI_X_SENSITIVITY, MI_X_SENSITIVITY_RESET, MI_Y_SENSITIVITY, MI_Y_SENSITIVITY_RESET, MI_COLD_MODE>;

class ScreenMenuSnakeSettings : public ScreenMenuSnakeSettings__ {
public:
    ScreenMenuSnakeSettings();
    virtual void windowEvent(window_t *sender, GUI_event_t ev, void *param) override;
};

/*********************************************************************************/

using ScreenMenuSnakeTuneSettings__ = ScreenMenu<EFooter::On, MI_RETURN,
#if PRINTER_IS_PRUSA_MINI()
    MI_BRIGHTNESS,
#endif
    MI_MBL_VALUES, MI_SKEW_XY, MI_SKEW_XZ, MI_SKEW_YZ, MI_COLD_MODE>;

class ScreenMenuSnakeTuneSettings : public ScreenMenuSnakeTuneSettings__ {
public:
    ScreenMenuSnakeTuneSettings();
    virtual void windowEvent(window_t *sender, GUI_event_t ev, void *param) override;
};
