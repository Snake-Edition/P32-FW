/**
 * @file screen_menu_print_head_profiles.hpp
 */

#pragma once

#include "screen_menu.hpp"
#include "MItem_print_head_profiles.hpp"
#include "i18n.h"
#include "window_header.hpp"

using ScreenMenuPrintHeadProfiles__ = ScreenMenu<GuiDefaults::MenuFooter,
    MI_RETURN,
    MI_HEAD_CURRENT,
    MI_HEAD_ORIGINAL_PRUSA,
    MI_HEAD_REVO_SIX
    >;

class ScreenMenuPrintHeadProfiles : public ScreenMenuPrintHeadProfiles__ {
public:
    constexpr static const char *label = N_("Print Head Profiles");
    ScreenMenuPrintHeadProfiles();

protected:
    void update_gui();
    virtual void windowEvent(window_t *sender, GUI_event_t event, void *param) override;
};
