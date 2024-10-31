#pragma once

#include <screen_menu.hpp>
#include <window_menu_virtual.hpp>
#include <WindowMenuItems.hpp>
#include <window_menu_callback_item.hpp>

namespace screen_factory_reset {

class WindowMenuFactoryReset final : public WindowMenuVirtual<MI_RETURN, WindowMenuCallbackItem> {
public:
    WindowMenuFactoryReset(window_t *parent, Rect16 rect);

public:
    int item_count() const final;

protected:
    void setup_item(ItemVariant &variant, int index) final;
};

} // namespace screen_factory_reset

class ScreenFactoryReset final : public ScreenMenuBase<screen_factory_reset::WindowMenuFactoryReset> {
public:
    ScreenFactoryReset();
};
