#pragma once
#include <optional>
#include <bitset>
namespace buddy {

/// Class for managing automatic retraction after print or load, so that the printer keeps the nozzle empty for MBL and non-printing to prevent oozing.
/// Only to be managed from the marlin thread
class AutoRetract {
    friend AutoRetract &auto_retract();

public:
    static constexpr float minimum_auto_retract_distance = 20.f; ///< Minimum retract distance for the filament to be considered auto-retracted. Auto-retracted filaments can be unloaded without heating.

    /// \returns whether the specified hotend is retracted (some amount > 0.0f) and is a known value -> will deretract on positive Z move
    bool will_deretract() const;

    /// \returns whether the specified \param hotend is retracted (some amount > 0.0f) and is a known value -> will deretract on positive Z move
    bool will_deretract(uint8_t hotend) const;

    /// \returns true if the filament is completely retracted from the \param hotend's nozzle (distance >= minimum_retract_distance), allowing for cold unload.
    bool is_safely_retracted_for_unload(uint8_t hotend) const;

    /// \returns true if the filament is completely retracted from the current hotend's nozzle (distance >= minimum_retract_distance), allowing for cold unload.
    bool is_safely_retracted_for_unload() const;

    /// How much is the filament retracted from the nozzle (mm), std::nullopt if retracted distance not a known value
    std::optional<float> retracted_distance() const;

    /// If !is_safely_retracted_for_unload(), executes the retraction process and saves retracted distance
    void maybe_retract_from_nozzle();

    /// If will_deretract(), executes the deretraction process and set retracted distance to unknown value (because it can be changed by printing moves without notice)
    void maybe_deretract_to_nozzle();

    /// Save values to persistent storage
    void set_retracted_distance(uint8_t hotend, std::optional<float> distance);

private:
    AutoRetract();

    /// Common checks for retract & deretract
    bool can_perform_action() const;

    /// Shadows the config_store variable to reduce mutex locking
    std::bitset<HOTENDS> retracted_hotends_bitset_ = 0;

    bool is_checking_deretract_ = false;
};

AutoRetract &auto_retract();

} // namespace buddy
