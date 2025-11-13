/// @file
#pragma once

#include <option/has_chamber_vents.h>

static_assert(HAS_CHAMBER_VENTS());

namespace automatic_chamber_vents {

/// @brief Opens the printer's vent grille.
/// @return true if the operation was successful, false otherwise.
[[nodiscard]] bool open();

/// @brief Closes the printer's vent grille.
/// @return true if the operation was successful, false otherwise.
[[nodiscard]] bool close();

} // namespace automatic_chamber_vents
