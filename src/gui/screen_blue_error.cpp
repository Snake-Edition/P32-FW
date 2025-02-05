
#include "screen_blue_error.hpp"
#include <ScreenHandler.hpp>
#include <sound.hpp>
#include <support_utils.h>
#include <version.h>
#include <crash_dump/dump.hpp>
#if HAS_LEDS()
    #include <leds/status_leds_handler.hpp>
#endif

using namespace crash_dump;

static const constexpr Rect16 fw_version_rect = GuiDefaults::EnableDialogBigLayout ? Rect16(30, ScreenBlueError::fw_line_top, GuiDefaults::ScreenWidth - 30, 20) : Rect16(6, 295, GuiDefaults::ScreenWidth - 6, 13);
static const constexpr Rect16 header_rect = GuiDefaults::EnableDialogBigLayout ? Rect16(14, 10, 240, GuiDefaults::HeaderHeight - 10) : GuiDefaults::RectHeader;
static const constexpr Rect16 title_rect = GuiDefaults::EnableDialogBigLayout ? Rect16(30, 44, GuiDefaults::ScreenWidth - 60, 20) : Rect16(13, 12, GuiDefaults::ScreenWidth - 26, 20);

ScreenBlueError::ScreenBlueError()
    : ScreenResetError(fw_version_rect)
    ///@note No translations on blue screens.
    , header(this, header_rect, is_multiline::no, is_closed_on_click_t::no, string_view_utf8::MakeCPUFLASH("UNKNOWN ERROR"))
    , title(this, title_rect, is_multiline::no, is_closed_on_click_t::no, string_view_utf8::MakeCPUFLASH("Unable to show details"))
    , description(this, description_rect, is_multiline::yes) {
    SetBlueLayout();

    // Simple text instead of header
    header.SetAlignment(Align_t::LeftTop());
    if constexpr (GuiDefaults::EnableDialogBigLayout) {
        header.set_font(Font::special);
    } else {
        header.set_font(Font::small);
    }

    description.set_font(description_font);
    description.set_check_overflow(false);

#if HAS_LEDS()
    leds::StatusLedsHandler::instance().set_error();
#endif
}
