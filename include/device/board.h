#pragma once
///
/// @file board.h
///
/// Macros to differentiate between our Buddy boards
///
/// Usage:
///
///    #if BOARD_IS_BUDDY()
///       ... code ...
///    #elif BOARD_IS_XBUDDY()
///       ... code ...
///    #else
///       #error Unsupported board
///    #endif
///
///
///    #if BOARD_IS_BUDDY()
///       ... code ...
///    #elif BOARD_IS_XBUDDY()
///       ... code ...
///    #else
///       #error Unsupported board
///    #endif
///
///
/// Macros to be defined when invoking the compiler
/// DO NOT USE those macros in your code (if you don't have a really good reason to).
/// USE the macros defined below instead.
///
/// BOARD (e.g. BUDDY_BOARD)
///

#define BOARD_BUDDY            1
#define BOARD_XBUDDY           2
#define BOARD_XLBUDDY          3
#define BOARD_DWARF            4
#define BOARD_MODULARBED       5
#define BOARD_XL_DEV_KIT_XLB   6
#define BOARD_XBUDDY_EXTENSION 7

#if defined(BOARD) && BOARD == BOARD_BUDDY
    #define BOARD_IS_BUDDY() 1
    #define BOARD_STRING()   "Buddy"
#elif defined(BOARD) && BOARD == BOARD_XBUDDY
    #define BOARD_IS_XBUDDY() 1
    #define BOARD_STRING()    "XBuddy"
#elif defined(BOARD) && BOARD == BOARD_XLBUDDY
    #define BOARD_IS_XLBUDDY() 1
    #define BOARD_STRING()     "XLBuddy"
#elif defined(BOARD) && BOARD == BOARD_DWARF
    #define BOARD_IS_DWARF() 1
    #define BOARD_STRING()   "Dwarf"
#elif defined(BOARD) && BOARD == BOARD_MODULARBED
    #define BOARD_IS_MODULARBED() 1
    #define BOARD_STRING()        "ModularBed"
#elif defined(BOARD) && BOARD == BOARD_XL_DEV_KIT_XLB
    #define BOARD_IS_XLBUDDY()        1 // todo: remove, for now xl dev two  boards enabled
    #define BOARD_IS_XL_DEV_KIT_XLB() 1
    #define BOARD_STRING()            "XL_DEV_KIT_XLB"
#elif defined(BOARD) && BOARD == BOARD_XBUDDY_EXTENSION
    #define BOARD_IS_XBUDDY_EXTENSION() 1
    #define BOARD_STRING()              "XBUDDY_EXTENSION"
#else
    #error Please define the BOARD macro
#endif

#ifndef BOARD_IS_BUDDY
    #define BOARD_IS_BUDDY() 0
#endif

#ifndef BOARD_IS_XBUDDY
    #define BOARD_IS_XBUDDY() 0
#endif

#ifndef BOARD_IS_XLBUDDY
    #define BOARD_IS_XLBUDDY() 0
#endif

#ifndef BOARD_IS_XL_DEV_KIT_XLB
    #define BOARD_IS_XL_DEV_KIT_XLB() 0
#endif

#ifndef BOARD_IS_DWARF
    #define BOARD_IS_DWARF() 0
#endif

#ifndef BOARD_IS_MODULARBED
    #define BOARD_IS_MODULARBED() 0
#endif

#ifndef BOARD_IS_XBUDDY_EXTENSION
    #define BOARD_IS_XBUDDY_EXTENSION() 0
#endif
