/**
 * @file font_flags.hpp
 * @author Radek Vana
 * @brief definition of font render flags
 * @date 2021-02-02
 */
#pragma once

#include "align.hpp"
#include <stdint.h>

enum class is_multiline : bool { no,
    yes };
enum class check_overflow : bool { no,
    yes };

struct text_flags {
    Align_t align;
    is_multiline multiline;
    check_overflow overflow;

    text_flags(Align_t align, is_multiline multiline = is_multiline::no, check_overflow overflow = check_overflow::yes)
        : align(align)
        , multiline(multiline)
        , overflow(overflow) {}
};
