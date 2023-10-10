// screen_menu_preheat.cpp

#include "screen_menus.hpp"
#include "screen_menu.hpp"
#include "filament.h"
#include "filament.hpp"
#include "marlin_client.hpp"
#include "screen_menu.hpp"
#include "MItem_tools.hpp"
#include "i18n.h"
#include "ScreenHandler.hpp"
#include "temperature.h"

using Screen = ScreenMenu<EFooter::On, MI_RETURN,
    MI_Filament<FILAMENT_PLA>,
    MI_Filament<FILAMENT_PETG>,
    MI_Filament<FILAMENT_ASA>,
    MI_Filament<FILAMENT_PC>,
    MI_Filament<FILAMENT_PVB>,
    MI_Filament<FILAMENT_ABS>,
    MI_Filament<FILAMENT_HIPS>,
    MI_Filament<FILAMENT_PP>,
    MI_Filament<FILAMENT_FLEX>,
    MI_Filament<FILAMENT_NONE>>;

int8_t one_click_preheat = true;

class ScreenMenuPreheat : public Screen {
public:
    constexpr static const char *label = N_("PREHEAT");
    ScreenMenuPreheat()
        : Screen(_(label)) {
        if (one_click_preheat && Filaments::CurrentIndex() != filament_t::NONE) {
            thermalManager.setTargetHotend(Filaments::Current().nozzle_preheat, 0);
            Screens::Access()->Close();
        }
        one_click_preheat = !one_click_preheat;
    }
};

ScreenFactory::UniquePtr GetScreenMenuPreheat() {
    return ScreenFactory::Screen<ScreenMenuPreheat>();
}

using ScreenNoRet = ScreenMenu<EFooter::On, HelpLines_None,
    MI_Filament<FILAMENT_PLA>,
    MI_Filament<FILAMENT_PETG>,
    MI_Filament<FILAMENT_ASA>,
    MI_Filament<FILAMENT_PC>,
    MI_Filament<FILAMENT_PVB>,
    MI_Filament<FILAMENT_ABS>,
    MI_Filament<FILAMENT_HIPS>,
    MI_Filament<FILAMENT_PP>,
    MI_Filament<FILAMENT_FLEX>,
    MI_Filament<FILAMENT_NONE>>;

class ScreenMenuPreheatNoRet : public ScreenNoRet {
public:
    constexpr static const char *label = N_("PREHEAT");
    ScreenMenuPreheatNoRet()
        : ScreenNoRet(_(label)) {}
};

ScreenFactory::UniquePtr GetScreenMenuPreheatNoRet() {
    return ScreenFactory::Screen<ScreenMenuPreheatNoRet>();
}
