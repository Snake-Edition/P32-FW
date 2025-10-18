/**
 * @file MItem_extruder_profiles.hpp
 * @brief Menu items for extruder profiles in Hardware menu
 */

#pragma once
#include "WindowMenuItems.hpp"
#include "i18n.h"
#include "config_store/store_instance.hpp"

// Extruder profile definitions
enum class ExtruderProfile : uint8_t {
    Standard = 0, // Standard Prusa extruder
    BondTech = 1, // BondTech extruder
    Binus_Dualdrive = 2, // Binus Dualdrive https://www.printables.com/model/946290-dual-gear-drive-extruder-for-prusa-minimini#bom-bill-of-materials-
    Custom = 3 // Custom settings
};

// Profile data structure
struct ExtruderProfileData {
    const char *name;
    float steps_per_unit_e; // E steps per mm
    bool reverse_direction; // Whether to reverse extruder direction
    float max_feedrate_e; // Max E feedrate
    float max_acceleration_e; // Max E acceleration
    float max_jerk_e; // Max E jerk
    float retract_length; // Default retraction length
    float retract_speed; // Default retraction speed
};

// Profile database
extern const ExtruderProfileData extruder_profiles[];
extern const size_t extruder_profiles_count;

// Main extruder profiles menu item
class MI_EXTRUDER_PROFILES : public IWindowMenuItem {
    static constexpr const char *const label = N_("Extruder Profiles");

public:
    MI_EXTRUDER_PROFILES();
    virtual ~MI_EXTRUDER_PROFILES();

protected:
    virtual void click(IWindowMenu &window_menu) override;
};

// Profile selection items
class MI_PROFILE_STANDARD : public IWindowMenuItem {
    static constexpr const char *const label = N_("Prusa");

public:
    MI_PROFILE_STANDARD();

protected:
    virtual void click(IWindowMenu &window_menu) override;
};

class MI_PROFILE_BONDTECH : public IWindowMenuItem {
    static constexpr const char *const label = N_("BondTech");

public:
    MI_PROFILE_BONDTECH();

protected:
    virtual void click(IWindowMenu &window_menu) override;
};

class MI_PROFILE_BINUS_DUALDRIVE : public IWindowMenuItem {
    static constexpr const char *const label = N_("Binus Dualdrive");

public:
    MI_PROFILE_BINUS_DUALDRIVE();

protected:
    virtual void click(IWindowMenu &window_menu) override;
};

// Opens Experimental settings menu for manual tuning
class MI_PROFILE_CUSTOM : public IWindowMenuItem {
    static constexpr const char *const label = N_("Custom");

public:
    MI_PROFILE_CUSTOM();
    virtual ~MI_PROFILE_CUSTOM();

protected:
    virtual void click(IWindowMenu &window_menu) override;
};

// Current profile display
class MI_CURRENT_PROFILE : public WI_INFO_t {
    static constexpr const char *const label = N_("Current");

public:
    MI_CURRENT_PROFILE();
    static const char *get_current_profile_name();

protected:
    virtual void click(IWindowMenu &window_menu) override;
};
