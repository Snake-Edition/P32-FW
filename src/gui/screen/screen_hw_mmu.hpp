#pragma once

#include <gui/menu_item/specific/menu_items_hw_mmu.hpp>
#include <screen_menu.hpp>

using ScreenMenuHwMmu_ = ScreenMenu<EFooter::Off,
    MI_RETURN,
    MI_MMU_FRONT_PTFE_LENGTH //
    >;

class ScreenMenuHwMmu final : public ScreenMenuHwMmu_ {
public:
    ScreenMenuHwMmu();
};
