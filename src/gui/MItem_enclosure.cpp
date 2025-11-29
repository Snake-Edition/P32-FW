
#include "MItem_enclosure.hpp"
#include "img_resources.hpp"
#include "WindowMenuSpin.hpp"
#include "xl_enclosure.hpp"
#include "ScreenHandler.hpp"
#include "screen_change_filter.hpp"
#include <screen_menu_enclosure.hpp>
#include <window_dlg_wait.hpp>

/* Once is Enclosure enabled in menu with ON/OFF switch (MI_ENCLOSURE_ENABLED), it tests the fan and after that Enclosure is declared Active */
/* If test was passed, MI_ENCLOSURE_ENABLE is swapped with MI_ENCLOSURE and enclosure settings can be accessed */
/* This hides enclosure settings for Users without enclosure */
MI_ENCLOSURE::MI_ENCLOSURE()
    : IWindowMenuItem(_(label), nullptr, is_enabled_t::yes, xl_enclosure.isActive() ? is_hidden_t::no : is_hidden_t::yes, expands_t::yes) {
}

void MI_ENCLOSURE::click(IWindowMenu & /*window_menu*/) {
    Screens::Access()->Open(ScreenFactory::Screen<ScreenMenuEnclosure>);
}

MI_ENCLOSURE_ENABLE::MI_ENCLOSURE_ENABLE()
    : WI_ICON_SWITCH_OFF_ON_t(xl_enclosure.isEnabled(), _(label), nullptr, is_enabled_t::yes, xl_enclosure.isActive() ? is_hidden_t::yes : is_hidden_t::no) {
    /* Swapping of Menu Items MI_ENCLOSURE_ENABLE and MI_ENCLOSURE (See comment above) */
}

void MI_ENCLOSURE_ENABLE::OnChange([[maybe_unused]] size_t old_index) {
    xl_enclosure.setEnabled(value());
    if (value()) {
        /* Wait until enclosure is initialized & ready (test takes 3s). If initialization fails, the enclosure gets disabled internally. */
        gui_dlg_wait([] {
            if (xl_enclosure.isActive() || !xl_enclosure.isEnabled()) {
                Screens::Access()->Close();
            }
        },
            _(wait_str));
        if (!xl_enclosure.isEnabled()) {
            set_value(false);
        }
    }
}
