#pragma once
#include "WindowMenuItems.hpp"
#include "WindowItemFormatableLabel.hpp"

class MI_ENCLOSURE : public IWindowMenuItem {
    constexpr static const char *label = N_("Enclosure Settings");

public:
    MI_ENCLOSURE();

protected:
    virtual void click(IWindowMenu &windowMenu) override;
};

class MI_ENCLOSURE_ENABLE : public WI_ICON_SWITCH_OFF_ON_t {
    static constexpr const char *const label = N_("Enclosure");
    static constexpr const char *const wait_str = N_("Testing enclosure fan");

public:
    MI_ENCLOSURE_ENABLE();

protected:
    void OnChange(size_t old_index) override;
};
