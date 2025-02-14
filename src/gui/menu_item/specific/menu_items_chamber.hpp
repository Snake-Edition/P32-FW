#pragma once

#include <WindowMenuItems.hpp>
#include <WindowItemFormatableLabel.hpp>
#include <WindowMenuSpin.hpp>

class MI_CHAMBER_TARGET_TEMP : public WiSpin {
public:
    MI_CHAMBER_TARGET_TEMP(const char *label = N_("Chamber Temperature"));

protected:
    virtual void OnClick() override;
    virtual void Loop() override;
};

class MI_CHAMBER_TEMP : public MenuItemAutoUpdatingLabel<float> {
public:
    MI_CHAMBER_TEMP(const char *label = N_("Chamber Temperature"));
};

class MI_PREHEAT_CHAMBER : public WI_ICON_SWITCH_OFF_ON_t {
public:
    MI_PREHEAT_CHAMBER();

protected:
    void OnChange(size_t) override;
};
