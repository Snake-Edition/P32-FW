#include "screen_filament_detail.hpp"

#include <filament_list.hpp>
#include <filament_gui.hpp>
#include <numeric_input_config_common.hpp>
#include <algorithm_extensions.hpp>
#include <dialog_text_input.hpp>
#include <ScreenHandler.hpp>
#include <utils/string_builder.hpp>
#include <screen/screen_preheat.hpp>

#if HAS_CHAMBER_API()
    #include <feature/chamber/chamber.hpp>
#endif

using namespace buddy;
using namespace screen_filament_detail;

// *MI_TOGGLE
MI_TOGGLE::MI_TOGGLE(Parameter param, const char *label)
    : WI_ICON_SWITCH_OFF_ON_t(false, _(label))
    , param_(param) {
}
void MI_TOGGLE::set_filament_type(FilamentType set) {
    filament_type = set;
    // The XOR inverts the value if `invert_value == true`
    set_value(filament_type.parameters().*param_ ^ invert_value);
    set_enabled(filament_type.is_customizable());
}
void MI_TOGGLE::OnChange(size_t) {
    // The XOR inverts the value if `invert_value == true`
    filament_type.modify_parameters([&](auto &p) { p.*param_ = (value() ^ invert_value); });
}

void MI_TOGGLE::set_invert_value(bool set) {
    // Should only be called during initialization
    assert(filament_type == FilamentType::none);
    invert_value = set;
}

// * MI_FILAMENT_NAME
MI_FILAMENT_NAME::MI_FILAMENT_NAME()
    : WiInfo(_("Name")) {}

void MI_FILAMENT_NAME::set_filament_type(FilamentType set) {
    filament_type = set;
    ArrayStringBuilder<GetInfoLen()> sb;
    filament_type.build_name_with_info(sb);
    ChangeInformation(sb.str());
    set_enabled(filament_type.is_customizable());
}

void MI_FILAMENT_NAME::click(IWindowMenu &) {
    FilamentTypeParameters::Name buf = filament_type.parameters().name;

    while (true) {
        if (!DialogTextInput::exec(GetLabel(), buf)) {
            return;
        }

        for (char *chp = buf.data(); *chp; chp++) {
            *chp = toupper(*chp);
        }

        if (const auto r = filament_type.can_be_renamed_to(buf.data()); !r) {
            MsgBoxWarning(_(r.error()), Responses_Ok);
            continue;
        }

        break;
    }

    filament_type.modify_parameters([&](auto &p) { memcpy(p.name.data(), buf.data(), buf.size()); });
    set_filament_type(filament_type);
}

// * MI_FILAMENT_NOZZLE_TEMPERATURE
MI_FILAMENT_NOZZLE_TEMPERATURE::MI_FILAMENT_NOZZLE_TEMPERATURE()
    : MI_SPIN(&FilamentTypeParameters::nozzle_temperature, numeric_input_config::filament_nozzle_temperature, HAS_MINI_DISPLAY() ? N_("Nozzle Temp") : N_("Nozzle Temperature")) {}

// * MI_FILAMENT_NOZZLE_PREHEAT_TEMPERATURE
MI_FILAMENT_NOZZLE_PREHEAT_TEMPERATURE::MI_FILAMENT_NOZZLE_PREHEAT_TEMPERATURE()
    : MI_SPIN(&FilamentTypeParameters::nozzle_preheat_temperature, numeric_input_config::nozzle_temperature, HAS_MINI_DISPLAY() ? N_("Preheat Temp") : N_("Nozzle Preheat Temperature")) {}

// * MI_FILAMENT_BED_TEMPERATURE
MI_FILAMENT_BED_TEMPERATURE::MI_FILAMENT_BED_TEMPERATURE()
    : MI_SPIN(&FilamentTypeParameters::heatbed_temperature, numeric_input_config::bed_temperature, HAS_MINI_DISPLAY() ? N_("Bed Temp") : N_("Bed Temperature")) {}

#if HAS_FILAMENT_HEATBREAK_PARAM()
MI_FILAMENT_HEATBREAK_TEMPERATURE::MI_FILAMENT_HEATBREAK_TEMPERATURE()
    : MI_SPIN(&FilamentTypeParameters::heatbreak_temperature, numeric_input_config::heatbreak_temperature, HAS_MINI_DISPLAY() ? N_("Heatbreak Temp") : N_("Heatbreak Temperature")) {}
#endif

#if HAS_CHAMBER_API()
namespace {
void setup_chamber_temp_item(IWindowMenuItem &item) {
    const auto caps = chamber().capabilities();
    item.set_is_hidden(!caps.temperature_control() && !caps.always_show_temperature_control);
}
} // namespace

// * MI_FILAMENT_MIN_CHAMBER_TEMPERATURE
MI_FILAMENT_MIN_CHAMBER_TEMPERATURE::MI_FILAMENT_MIN_CHAMBER_TEMPERATURE()
    : MI_SPIN(&FilamentTypeParameters::chamber_min_temperature, numeric_input_config::chamber_temp_with_none(), N_("Minimum Chamber Temperature")) {
    setup_chamber_temp_item(*this);
}

// * MI_FILAMENT_MAX_CHAMBER_TEMPERATURE
MI_FILAMENT_MAX_CHAMBER_TEMPERATURE::MI_FILAMENT_MAX_CHAMBER_TEMPERATURE()
    : MI_SPIN(&FilamentTypeParameters::chamber_max_temperature, numeric_input_config::chamber_temp_with_none(), N_("Maximum Chamber Temperature")) {
    setup_chamber_temp_item(*this);
}

// * MI_FILAMENT_TARGET_CHAMBER_TEMPERATURE
MI_FILAMENT_TARGET_CHAMBER_TEMPERATURE::MI_FILAMENT_TARGET_CHAMBER_TEMPERATURE()
    : MI_SPIN(&FilamentTypeParameters::chamber_target_temperature, numeric_input_config::chamber_temp_with_none(), N_("Nominal Chamber Temperature")) {
    setup_chamber_temp_item(*this);
}
#endif

// * MI_FILAMENT_REQUIRES_FILTRATION
#if HAS_CHAMBER_API()
MI_FILAMENT_REQUIRES_FILTRATION::MI_FILAMENT_REQUIRES_FILTRATION()
    : MI_TOGGLE(&FilamentTypeParameters::requires_filtration, N_("Requires Filtration")) {}

#endif

// * MI_FILAMENT_IS_ABRASIVE
MI_FILAMENT_IS_ABRASIVE::MI_FILAMENT_IS_ABRASIVE()
    : MI_TOGGLE(&FilamentTypeParameters::is_abrasive, N_("Is Abrasive")) {}

// * MI_FILAMENT_IS_FLEXIBLE
MI_FILAMENT_IS_FLEXIBLE::MI_FILAMENT_IS_FLEXIBLE()
    : MI_TOGGLE(&FilamentTypeParameters::is_flexible, N_("Is Flexible")) {}

// * MI_FILAMENT_VISIBLE
MI_FILAMENT_VISIBLE::MI_FILAMENT_VISIBLE()
    : WI_ICON_SWITCH_OFF_ON_t(false, _("Visible")) {
}

void MI_FILAMENT_VISIBLE::set_filament_type(FilamentType set) {
    filament_type = set;
    set_value(filament_type.is_visible());
    set_is_hidden(filament_type.is_visibility_customizable());
}

void MI_FILAMENT_VISIBLE::OnChange(size_t) {
    filament_type.set_visible(value());
}

// * MI_CONFIRM
MI_CONFIRM::MI_CONFIRM()
    : IWindowMenuItem(_("Confirm"), &img::ok_16x16) {}

void MI_CONFIRM::click(IWindowMenu &) {
    callback();
}

// * ScreenFilamentDetail
ScreenFilamentDetail::ScreenFilamentDetail(FilamentType filament_type)
    : ScreenFilamentDetail(filament_type, N_("FILAMENT DETAIL")) {}

ScreenFilamentDetail::ScreenFilamentDetail(PreheatModeParams params)
    : ScreenFilamentDetail(PendingAdHocFilamentType {}, N_("CUSTOM PARAMETERS")) {

    auto &confirm_item = Item<MI_CONFIRM>();
    confirm_item.set_is_hidden(false);
    confirm_item.callback = [extruder = params.target_extruder] {
        if (preheat_menu::WindowMenuPreheat::handle_filament_selection(PendingAdHocFilamentType {}, extruder)) {
            Screens::Access()->Close();
        }
    };
}

ScreenFilamentDetail::ScreenFilamentDetail(FilamentType filament_type, const char *title)
    : ScreenMenu(_(title)) {
    stdext::visit_tuple(container.menu_items, [&]<typename T>(T &item) {
        if constexpr (!std::is_same_v<T, MI_RETURN> && !std::is_same_v<T, MI_CONFIRM>) {
            item.set_filament_type(filament_type);
        };
    });

    Item<MI_CONFIRM>().set_is_hidden();
}
