/// @file
#include <window_msgbox_happy_printing.hpp>

#include <window_msgbox.hpp>

void MsgBoxHappyPrinting() {
    MsgBoxPepaCentered(
        _("Happy printing!"),
        { Response::Continue, Response::_none, Response::_none, Response::_none });
}
