#include "screen_hardfault.hpp"
#include "config.h"
#include "ScreenHandler.hpp"
#include "bsod_gui.hpp"
#include <find_error.hpp>
#include "sound.hpp"

using namespace bsod_details;

ScreenHardfault::ScreenHardfault()
    : ScreenBlueError() {
    ///@note No translations on blue screens.

    header.SetText(string_view_utf8::MakeCPUFLASH("HARDFAULT"));

    // Show reason of hardfault as title
    title.SetText(string_view_utf8::MakeCPUFLASH(get_hardfault_reason()));

    char *buffer;

    // Show last known task, core registers and stack as description
    buffer = txt_err_description;
    get_stack(buffer, get_regs(buffer, get_task_name(buffer, std::size(txt_err_description))));
    description.SetText(string_view_utf8::MakeRAM(txt_err_description));
}
