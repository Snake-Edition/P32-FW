/**
 * @file window_thumbnail.cpp
 */

#include "window_thumbnail.hpp"
#include "gcode_reader_interface.hpp"
#include "display.hpp"

class GCodeQOIReader final : public AbstractByteReader {
private:
    IGcodeReader *reader;

public:
    explicit GCodeQOIReader(IGcodeReader *reader_)
        : reader { reader_ } {}

    std::span<std::byte> read(std::span<std::byte> buffer) final {
        // TODO implement reading multiple bytes at a time
        size_t n = buffer.size();
        size_t pos = 0;
        auto data = buffer.data();
        while (n != pos) {
            char c;
            if (reader->stream_getc(c) == IGcodeReader::Result_t::RESULT_OK) {
                data[pos++] = (std::byte)c;
            } else {
                break;
            }
        }
        return { data, pos };
    }
};

//-------------------------- Thumbnail --------------------------------------

WindowThumbnail::WindowThumbnail(window_t *parent, Rect16 rect)
    : window_icon_t(parent, rect, nullptr) {
}

//------------------------- Preview Thumbnail ------------------------------------

WindowPreviewThumbnail::WindowPreviewThumbnail(window_t *parent, Rect16 rect)
    : WindowThumbnail(parent, rect) {
}

void WindowPreviewThumbnail::unconditionalDraw() {
    gcode_reader = AnyGcodeFormatReader { GCodeInfo::getInstance().GetGcodeFilepath() };
    if (!gcode_reader.is_open()) {
        return;
    }

    if (!gcode_reader->stream_thumbnail_start(Width(), Height(), IGcodeReader::ImgType::QOI)) {
        return;
    }

    GCodeQOIReader res { gcode_reader.get() };
    display::draw_img(point_ui16(Left(), Top()), res);
}

//------------------------- Progress Thumbnail -----------------------------------

WindowProgressThumbnail::WindowProgressThumbnail(window_t *parent, Rect16 rect, size_t allowed_old_thumbnail_width)
    : WindowThumbnail(parent, rect)
    , redraw_whole(true)
    , old_allowed_width(allowed_old_thumbnail_width) {
    gcode_reader = AnyGcodeFormatReader { GCodeInfo::getInstance().GetGcodeFilepath() };
}

Rect16::Left_t WindowProgressThumbnail::get_old_left() {
    assert(old_allowed_width < Width()); // currently unsupported otherwise
    const auto cur_rect { GetRect() };
    // center image by moving left by half of the difference of the widths
    return cur_rect.Left() + (cur_rect.Width() - old_allowed_width) / 2;
}

void WindowProgressThumbnail::unconditionalDraw() {
    if (!gcode_reader.is_open()) {
        return;
    }

    // TODO: check if redraw_whole is still needed, Invalidate might be enough now
    if (!redraw_whole) { // No longer drawing image per-progress, so draw is only valid if the whole image wants to be drawn
        return;
    }

    bool have_old_alternative { false };

    if (!gcode_reader->stream_thumbnail_start(Width(), Height(), IGcodeReader::ImgType::QOI)) {
        if (old_allowed_width < Width() && !gcode_reader->stream_thumbnail_start(old_allowed_width, Height(), IGcodeReader::ImgType::QOI)) {
            return;
        } else {
            have_old_alternative = true;
        }
    }

    // Draw whole thumbnail:
    GCodeQOIReader res { gcode_reader.get() };
    display::draw_img(point_ui16(have_old_alternative ? get_old_left() : Left(), Top()), res);

    redraw_whole = false;
}

void WindowProgressThumbnail::pauseDeinit() {
    gcode_reader = AnyGcodeFormatReader {};
}

void WindowProgressThumbnail::pauseReinit() {
    gcode_reader = AnyGcodeFormatReader { GCodeInfo::getInstance().GetGcodeFilepath() };
}

void WindowProgressThumbnail::redrawWhole() {
    redraw_whole = true;
}
