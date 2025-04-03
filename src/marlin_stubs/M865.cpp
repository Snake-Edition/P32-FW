#include "PrusaGcodeSuite.hpp"

#include <option/has_filament_heatbreak_param.h>

#include <filament.hpp>
#include <temperature.hpp>
#include <utils/string_builder.hpp>

/** \addtogroup G-Codes
 * @{
 */

/**
 *### M865: Configure filament parameters
 *
 *#### Parameters
 * - `I<ix>` - Configure parameters of a Custom filament currently loaded to the specified tool (indexed from 0)
 * - `U<ix>` - Configure parameters of a User filament (indexed from 0)
 * - `X` - Configure parameters of a Custom filament type that will be loaded using `M600 F"##"` (or similar filament change gcode)
 * - `F"<preset>"` - Configure parameters of User filament with this name  (or select Preset filament for `L`)
 *
 * - `L<ix>` - Set currently loaded filament for the given tool to the selected filament
 *
 * - `R` - Reset parameters not specified in this gcode to defaults
 *
 * - `T` - Nozzle temperature
 * - `P` - Nozzle preheat temperature
 * - `B` - Bed temperature
 * - `H` - Heatbreak temperature
 * - `A` - Is abrasive
 * - `G` - Is flexible
 *
 * - `C` - Target chamber temperature
 * - `D` - Minimum chamber temperature
 * - `E` - Maximum chamber temperature
 * - `F` - Requries filtration
 *
 * - `N"<string>"` - New filament name
 *
 * Ad-hoc/custom filaments can the be referenced in other gcodes using adhoc_filament_gcode_prefix.
 * For example `M600 S"#0"` will load ad-hoc filament previously set with `M865 I0`.
 */
void PrusaGcodeSuite::M865() {
    GCodeParser2 p;
    if (!p.parse_marlin_command()) {
        return;
    }

    FilamentType filament_type;

    if (const auto slot = p.option<uint8_t>('I', static_cast<uint8_t>(0), static_cast<uint8_t>(adhoc_filament_type_count - 1))) {
        filament_type = AdHocFilamentType { .tool = *slot };
        if (config_store().get_filament_type(*slot) != filament_type) {
            SERIAL_ERROR_MSG("The selected tool does not have the ad-hoc filament loaded. Changes will have no effect.");
        }

    } else if (p.option<bool>('X').value_or(false)) {
        filament_type = PendingAdHocFilamentType {};

    } else if (const auto slot = p.option<uint8_t>('U', static_cast<uint8_t>(0), static_cast<uint8_t>(user_filament_type_count - 1))) {
        filament_type = UserFilamentType { .index = *slot };

    } else if (const auto ft = p.option<FilamentType>('F')) {
        filament_type = *ft;

    } else {
        SERIAL_ERROR_MSG("Filament type invalid or not specified.");
        return;
    }

    FilamentTypeParameters params = filament_type.parameters();

    if (p.option<bool>('R').value_or(false)) {
        params = {};
    }

    p.store_option('T', params.nozzle_temperature);
    p.store_option('P', params.nozzle_preheat_temperature);
    p.store_option('B', params.heatbed_temperature);

    p.store_option('A', params.is_abrasive);
    p.store_option('G', params.is_flexible);

#if HAS_FILAMENT_HEATBREAK_PARAM()
    p.store_option('H', params.heatbreak_temperature);
#endif

#if HAS_CHAMBER_API()
    p.store_option('C', params.chamber_target_temperature);
    p.store_option('D', params.chamber_min_temperature);
    p.store_option('E', params.chamber_max_temperature);
    p.store_option('F', params.requires_filtration);
#endif

    std::array<char, filament_name_buffer_size - 1> name_buf;
    if (const auto opt = p.option<std::string_view>('N', name_buf)) {
        StringBuilder b(params.name);
        b.append_std_string_view(*opt);

        if (const auto r = filament_type.can_be_renamed_to(params.name.data()); !r) {
            SERIAL_ERROR_START();
            SERIAL_ECHOLN(r.error());
            return;
        }
    }

    if (filament_type.is_customizable()) {
        filament_type.set_parameters(params);
    }

    if (auto load = p.option<uint8_t>('L', static_cast<uint8_t>(0), static_cast<uint8_t>(EXTRUDERS))) {
        config_store().set_filament_type(*load, filament_type);
    }
}

/** @}*/
