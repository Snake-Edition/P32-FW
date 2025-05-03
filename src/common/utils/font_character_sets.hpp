#pragma once
#include <printers.h>
#include <option/enable_translation_ja.h>
#include <option/enable_translation_uk.h>

/* We support different character sets for different fonts
 * Various character sets are used for various fonts to optimize the memory
 */
enum class FontCharacterSet : uint8_t {
#if not PRINTER_IS_PRUSA_MINI()
    full = 0, /// standard ASCII (32 - 127) + all required non-ascii latin + all required Katakana characters + japanese ',' + japanese '.' + cyrillic alphabet
    digits = 1, /// digits (0 - 9) + '.' + '?' + '%' + '-'
#else
    #if ENABLE_TRANSLATION_JA()
    latin_and_katakana = 2, /// standard ASCII <32;127> + all required non-ascii latin + all katakana characters <0x30A0;0x30FF> + japanese ',' + japanese '.'
    #elif ENABLE_TRANSLATION_UK()
    latin_and_cyrillic = 3, /// standard ASCII <32;127> + all required non-ascii latin + all cyrillic standard characters <0x0400;0x04FF>
    #else
    latin = 4, /// standard ASCII (32 - 127) + all required non-ascii latin
    #endif
#endif
};
