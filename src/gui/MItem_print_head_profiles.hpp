/**
 * @file MItem_print_head_profiles.hpp
 * @brief Menu items for Print Head Profiles (Input Shaper X presets)
 */

#pragma once

#include "WindowMenuItems.hpp"
#include "i18n.h"

// Main entry
class MI_PRINT_HEAD_PROFILES : public IWindowMenuItem {
    static constexpr const char *const label = N_("Print Head Profiles");

public:
    MI_PRINT_HEAD_PROFILES();

protected:
    virtual void click(IWindowMenu &window_menu) override;
};

// "Original Prusa" -> restore default X-axis Input Shaper config
class MI_HEAD_ORIGINAL_PRUSA : public IWindowMenuItem {
    static constexpr const char *const label = N_("Original Prusa");

public:
    MI_HEAD_ORIGINAL_PRUSA();

protected:
    virtual void click(IWindowMenu &window_menu) override;
};

// "Revo Six" -> preset action (for now sets X-axis frequency to 0 and enables IS)
class MI_HEAD_REVO_SIX : public IWindowMenuItem {
    static constexpr const char *const label = N_("Revo Six");

public:
    MI_HEAD_REVO_SIX();

protected:
    virtual void click(IWindowMenu &window_menu) override;
};

// Current Print Head profile display
class MI_HEAD_CURRENT : public WI_INFO_t {
    static constexpr const char *const label = N_("Current");

public:
    MI_HEAD_CURRENT();
    static const char *get_current_head_profile_name();

protected:
    virtual void click(IWindowMenu &window_menu) override;
};
