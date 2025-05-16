#include "menu_items_hw_mmu.hpp"

#include <img_resources.hpp>
#include <marlin/Configuration.h>
#include <feature/prusa/MMU2/mmu2_mk4.h>

using namespace buddy;

static constexpr NumericInputConfig mmu_bowden_limits {
    .min_value = 341,
    .max_value = 1000,
    .unit = Unit::millimeter,
};

MI_MMU_FRONT_PTFE_LENGTH::MI_MMU_FRONT_PTFE_LENGTH()
    : WiSpin(MMU2::mmu2.BowdenLength(), mmu_bowden_limits, _("Front PTFE Length")) //
{
}

void MI_MMU_FRONT_PTFE_LENGTH::OnClick() {
    MMU2::mmu2.WriteRegister(std::to_underlying(MMU2::Register::Bowden_Length), value());
}
