#pragma once

enum class GuiLayout {
    red_screen,
    warning_dialog,
    mmu_dialog,
};

enum class DisplayOption {
    all,
    mini,
    large,
};

struct ErrorEntry {
    const char *title;
    const char *text;
    DisplayOption display;
    GuiLayout layout;
};
