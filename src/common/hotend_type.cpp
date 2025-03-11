#include "hotend_type.hpp"

#include <buddy/unreachable.hpp>

const char *hotend_type_name(HotendType t) {
    switch (t) {

    case HotendType::stock:
        return N_("Stock");

    case HotendType::stock_with_sock:
        return N_("With sock");

    case HotendType::e3d_revo:
        return N_("E3D Revo");

    case HotendType::_cnt:
        break;
    }

    BUDDY_UNREACHABLE();
};
