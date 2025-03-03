#pragma once

#include <i_window_menu_item.hpp>

class MI_CO_CANCEL_OBJECT : public IWindowMenuItem {
public:
    MI_CO_CANCEL_OBJECT();

protected:
    virtual void click(IWindowMenu &window_menu) override;
};
