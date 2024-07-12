// screen_mbl_values.cpp

#include "screen_mbl_values.hpp"
#include "ScreenHandler.hpp"
#include "../../lib/Marlin/Marlin/src/inc/MarlinConfig.h"
#include "../../lib/Marlin/Marlin/src/feature/bedlevel/bedlevel.h"
#include "display.hpp"

// extern bed_mesh_t z_values;

screen_mbl_values_t::screen_mbl_values_t()
    : screen_t(nullptr) {}

void screen_mbl_values_t::windowEvent(window_t * /*sender*/, GUI_event_t event, void * /*param*/) {
    if (event == GUI_event_t::CLICK) {
        Screens::Access()->Close();
    }
}

void screen_mbl_values_t::draw() {
    bed_mesh_t &z_values = unified_bed_leveling::z_values;
    float min_z = 0;
    float max_z = 0;
    for (int ix = 0; ix < GRID_MAJOR_POINTS_X; ++ix) {
        for (int iy = 0; iy < GRID_MAJOR_POINTS_Y; ++iy) {
            min_z = std::min(min_z, z_values[ix][iy]);
            max_z = std::max(max_z, z_values[ix][iy]);
        }
    }

    char text[30] = " ";
    render_text_align(Rect16(8, 10, 230, 290),
        string_view_utf8::MakeRAM((const uint8_t *)text), Font::small, COLOR_NAVY, COLOR_WHITE,
        { 0, 0, 0, 0 }, { Align_t::LeftTop(), is_multiline::yes });

    const int block_size = GuiDefaults::ScreenWidth / GRID_MAJOR_POINTS_Y;
    display::fill_rect(GuiDefaults::RectScreen, COLOR_BLACK); //< clear the display
    for (int ix = 0; ix < GRID_MAJOR_POINTS_X; ++ix) {
        for (int iy = 0; iy < GRID_MAJOR_POINTS_Y; ++iy) {
            const float num = z_values[ix][iy] - min_z; //< avoids negative numbers on screen
            snprintf(text, sizeof(text), "%1.3f", static_cast<double>(num));
            const int16_t x = ix * block_size;
            const int16_t y = (GRID_MAJOR_POINTS_Y - 1 - iy) * block_size;

            // TODO use hsl_to_rgb()
            const float extreme = std::clamp(1.f - std::min(num, max_z - z_values[ix][iy]) / (max_z - min_z) * 2.f, 0.f, 1.f);
            Color color;
            color.from_rgb(uint8_t(extreme * 0xff), uint8_t((1 - extreme) * 0xff), 0);
            display::draw_text(Rect16(x, y, block_size, block_size), string_view_utf8::MakeRAM((const uint8_t *)text), resource_font(Font::normal), COLOR_BLACK, color);
        }
    }

    // calculate and print X, Y tilt
    float x_tilt = 0;
    float y_tilt = 0;

    for (int iy = 0; iy < GRID_MAJOR_POINTS_Y; ++iy) {
        float avg = 0;
        for (int ix = 0; ix < GRID_MAJOR_POINTS_X; ++ix) {
            avg += z_values[ix][iy];
        }
        for (int ix = 0; ix < GRID_MAJOR_POINTS_X; ++ix) {
            x_tilt += (z_values[ix][iy] - avg) * ((GRID_MAJOR_POINTS_X - 1) / 2.f - ix);
        }
    }

    for (int ix = 0; ix < GRID_MAJOR_POINTS_X; ++ix) {
        float avg = 0;
        for (int iy = 0; iy < GRID_MAJOR_POINTS_Y; ++iy) {
            avg += z_values[ix][iy];
        }
        for (int iy = 0; iy < GRID_MAJOR_POINTS_Y; ++iy) {
            y_tilt += (z_values[ix][iy] - avg) * ((GRID_MAJOR_POINTS_Y - 1) / 2.f - iy);
        }
    }

    const float points = GRID_MAJOR_POINTS_X * GRID_MAJOR_POINTS_Y;
    x_tilt *= (GRID_MAJOR_POINTS_X - 1) / points;
    y_tilt *= (GRID_MAJOR_POINTS_Y - 1) / points;

    snprintf(text, sizeof(text), "X tilt %1.3f mm", static_cast<double>(x_tilt));
    display::draw_text(Rect16(10, GuiDefaults::RectScreen.Height() - 40, GuiDefaults::RectScreen.Width() - 10, 20), string_view_utf8::MakeRAM((const uint8_t *)text), resource_font(Font::normal), COLOR_BLACK, COLOR_WHITE);
    snprintf(text, sizeof(text), "Y tilt %1.3f mm", static_cast<double>(y_tilt));
    display::draw_text(Rect16(10, GuiDefaults::RectScreen.Height() - 20, GuiDefaults::RectScreen.Width() - 10, 20), string_view_utf8::MakeRAM((const uint8_t *)text), resource_font(Font::normal), COLOR_BLACK, COLOR_WHITE);
}
