#pragma once

#include <cstdint>
#include <optional>


struct M109Flags {
  float target_temp;
  bool wait_heat = true;            // Wait only when heating
  bool wait_heat_or_cool = false;   // Wait for heating or cooling
  bool autotemp = false;            // Use AUTOTEMP with S, B, F parameters
  std::optional<float> display_temp = std::nullopt; // If nullopt -> display nozzle temp
};

/**
* @brief Set extruder temperature and wait.
* 
* @param flags @see M109Flags
*/
void M109_no_parser(uint8_t target_extruder, const M109Flags& flags = {});
