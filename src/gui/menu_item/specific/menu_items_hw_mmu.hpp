#pragma once

#include <WindowItemFormatableLabel.hpp>
#include <WindowMenuSpin.hpp>

class MI_MMU_FRONT_PTFE_LENGTH : public WiSpin {
public:
    MI_MMU_FRONT_PTFE_LENGTH();

protected:
    virtual void OnClick() override;
};
