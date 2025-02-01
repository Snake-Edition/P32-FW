#include "gui.hpp"
#include "screen_menu.hpp"
#include "WindowMenuItems.hpp"
#include "MItem_menus.hpp"
#include "MItem_tools.hpp"

using ScreenMenuSnakeSettings__ = ScreenMenu<EFooter::On, MI_RETURN, MI_BRIGHTNESS, MI_MBL_VALUES, MI_SKEW_XY, MI_SKEW_XZ, MI_SKEW_YZ, MI_X_AXIS_LEN, MI_Y_AXIS_LEN, MI_COLD_MODE>;

class ScreenMenuSnakeSettings : public ScreenMenuSnakeSettings__ {
public:
    ScreenMenuSnakeSettings();
    virtual void windowEvent(window_t *sender, GUI_event_t ev, void *param) override;
};

/*********************************************************************************/

using ScreenMenuSnakeTuneSettings__ = ScreenMenu<EFooter::On, MI_RETURN, MI_BRIGHTNESS, MI_MBL_VALUES, MI_SKEW_XY, MI_SKEW_XZ, MI_SKEW_YZ>;

class ScreenMenuSnakeTuneSettings : public ScreenMenuSnakeTuneSettings__ {
public:
    ScreenMenuSnakeTuneSettings();
    virtual void windowEvent(window_t *sender, GUI_event_t ev, void *param) override;
};
