#include "gui.hpp"
#include "screen_menu.hpp"
#include "WindowMenuItems.hpp"
#include "MItem_menus.hpp"
#include "MItem_tools.hpp"
#include "menu_spin_config.hpp"

using ScreenMenuSnakeSettings__ = ScreenMenu<EFooter::On, MI_RETURN>;

class ScreenMenuSnakeSettings : public ScreenMenuSnakeSettings__ {
public:
    ScreenMenuSnakeSettings();
    virtual void windowEvent(EventLock /*has private ctor*/, window_t *sender, GUI_event_t ev, void *param) override;
};

/*********************************************************************************/

using ScreenMenuSnakeTuneSettings__ = ScreenMenu<EFooter::On, MI_RETURN>;

class ScreenMenuSnakeTuneSettings : public ScreenMenuSnakeTuneSettings__ {
public:
    ScreenMenuSnakeTuneSettings();
    virtual void windowEvent(EventLock /*has private ctor*/, window_t *sender, GUI_event_t ev, void *param) override;
};
