#include "hotend_type.hpp"

#include <buddy/unreachable.hpp>

#include <option/has_loadcell.h>

const char *hotend_type_name(HotendType t) {
    switch (t) {

    case HotendType::stock:
        return N_("Stock");

#if !PRINTER_IS_PRUSA_MINI()
    case HotendType::stock_with_sock:
        return N_("With sock");
#endif

#if PRINTER_IS_PRUSA_MK3_5()
    case HotendType::e3d_revo:
        return N_("E3D Revo");
#endif

    case HotendType::_cnt:
        break;
    }

    // This shouldn't happen, but if it does, let the firmware continue.
    // Might be due to mis-migration in config-store.
    assert(0);
    return nullptr;
}

int8_t hotend_type_heater_selftest_offset(HotendType t) {
    switch (t) {

    case HotendType::stock:
        return 0;

#if PRINTER_IS_PRUSA_MK3_5()
    case HotendType::stock_with_sock:
        return -25;
#elif HAS_LOADCELL() // Approximation of HAS_NEXTRUDER(), which we don't have
    case HotendType::stock_with_sock:
        return -20;
#endif

#if PRINTER_IS_PRUSA_MK3_5()
    case HotendType::e3d_revo:
        return 40;
#endif

    case HotendType::_cnt:
        break;
    }

    // This shouldn't happen, but if it does, let the firmware continue.
    // Might be due to mis-migration in config-store.
    assert(0);
    return 0;
}
