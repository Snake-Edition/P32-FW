/// @file
#include <window_msgbox_happy_printing.hpp>

#include <window_msgbox.hpp>
#include <config_store/store_instance.hpp>

void MsgBoxHappyPrinting() {
    if (config_store().happy_printing_seen.get()) {
        return;
    }

    MsgBoxPepaCentered(
        _("Happy printing!"),
        { Response::Continue, Response::_none, Response::_none, Response::_none });
    config_store().happy_printing_seen.set(true);
}
