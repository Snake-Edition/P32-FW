/// @file
#pragma once

#include <option/has_gearbox_alignment.h>

static_assert(HAS_GEARBOX_ALIGNMENT());

namespace PrusaGcodeSuite {

void M1979();

}
