#include "gui.hpp"
#include "screen_menu.hpp"
#include "WindowMenuItems.hpp"
#include "MItem_menus.hpp"
#include "MItem_tools.hpp"

using ScreenMenuChangeFilament__ = ScreenMenu<EFooter::On, MI_RETURN, MI_M600>;

class ScreenMenuM600 : public ScreenMenuChangeFilament__ {
public:
    ScreenMenuM600();
    virtual void windowEvent(window_t *sender, GUI_event_t event, void *param) override;
};
