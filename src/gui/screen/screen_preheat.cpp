#include "screen_preheat.hpp"

#include "img_resources.hpp"
#include "marlin_client.hpp"
#include "stdlib.h"
#include "i18n.h"
#include <limits>
#include <filament_gui.hpp>
#include <utils/string_builder.hpp>
#include <gui/screen/filament/screen_filament_detail.hpp>
#include <ScreenHandler.hpp>

using namespace preheat_menu;

// * MI_FILAMENT
MI_FILAMENT::MI_FILAMENT(FilamentType filament_type, uint8_t target_extruder)
    : WiInfo({}, nullptr, is_enabled_t::yes, is_hidden_t::no)
    , filament_type(filament_type)
    , target_extruder(target_extruder) //
{
    const auto filament_params = filament_type.parameters();
    filament_name = filament_params.name;

    FilamentTypeGUI::setup_menu_item(filament_type, filament_name, *this);

    ArrayStringBuilder<GetInfoLen()> sb;
    sb.append_printf("%3u/%-3u", filament_params.nozzle_temperature, filament_params.heatbed_temperature);
    ChangeInformation(sb.str());
}

void MI_FILAMENT::click(IWindowMenu &) {
    WindowMenuPreheat::handle_filament_selection(filament_type, target_extruder);
}

// * WindowMenuPreheat
WindowMenuPreheat::WindowMenuPreheat(window_t *parent, const Rect16 &rect)
    : WindowMenuVirtual(parent, rect, CloseScreenReturnBehavior::no) //
{
}

void WindowMenuPreheat::set_data(const PreheatData &data) {
    index_mapping.set_item_enabled<Item::return_>(data.has_return_option);
    index_mapping.set_item_enabled<Item::cooldown>(data.has_cooldown_option);

    // PreheatData might contain -1 for extruder index - that would screw things up
    extruder_index = data.extruder;
    if (extruder_index >= EXTRUDERS) {
        extruder_index = marlin_vars().active_extruder.get();
    }
    if (extruder_index >= EXTRUDERS) {
        extruder_index = 0;
    }

    update_list();
}

void WindowMenuPreheat::set_show_all_filaments(bool set) {
    if (show_all_filaments_ == set) {
        return;
    }

    const auto prev_focused_index = focused_item_index();
    show_all_filaments_ = set;
    update_list();
    move_focus_to_index(prev_focused_index);
}

bool WindowMenuPreheat::handle_filament_selection(FilamentType filament_type, uint8_t target_extruder) {
    const auto filament = filament_type.parameters();

    if (filament.is_abrasive && !config_store().nozzle_is_hardened.get().test(target_extruder)) {
        StringViewUtf8Parameters<filament_name_buffer_size + 1> params;
        if (MsgBoxWarning(_("Filament '%s' is abrasive, but you don't have a hardened nozzle installed. Do you really want to continue?").formatted(params, filament.name.data()), Responses_YesNo) != Response::Yes) {
            return false;
        }
    }

    marlin_client::FSM_response_variant(PhasesPreheat::UserTempSelection, FSMResponseVariant::make<FilamentType>(filament_type));
    return true;
}

void WindowMenuPreheat::update_list() {
    const GenerateFilamentListConfig config {
        .visible_only = !show_all_filaments_,
        .visible_first = true,
    };
    generate_filament_list(filament_list, config);
    index_mapping.set_section_size<Item::filament_section>(filament_list.size());
    index_mapping.set_item_enabled<Item::show_all>(!show_all_filaments_);
    setup_items();
}

void WindowMenuPreheat::setup_item(ItemVariant &variant, int index) {
    const auto mapping = index_mapping.from_index(index);
    switch (mapping.item) {

    case Item::return_: {
        const auto callback = [this] {
            Validate(); /// don't redraw since we leave the menu
            marlin_client::FSM_response(PhasesPreheat::UserTempSelection, Response::Abort);
        };
        variant.emplace<WindowMenuCallbackItem>(_("Return"), callback, &img::folder_up_16x16);
        break;
    }

    case Item::cooldown: {
        const auto callback = [] {
            marlin_client::FSM_response(PhasesPreheat::UserTempSelection, Response::Cooldown);
        };
        variant.emplace<WindowMenuCallbackItem>(_(get_response_text(Response::Cooldown)), callback);
        break;
    }

    case Item::show_all: {
        const auto callback = [this] {
            set_show_all_filaments(true);
        };
        variant.emplace<WindowMenuCallbackItem>(_("Show All"), callback);
        break;
    }

    case Item::filament_section:
        variant.emplace<MI_FILAMENT>(filament_list[mapping.pos_in_section], extruder_index);
        break;

    case Item::adhoc_filament: {
        const auto callback = [this] {
            const ScreenFilamentDetail::PreheatModeParams params {
                .target_extruder = extruder_index,
            };
            Screens::Access()->Open(ScreenFactory::ScreenWithArg<ScreenFilamentDetail>(params));
        };
        variant.emplace<WindowMenuCallbackItem>(_("Custom"), callback);
        break;
    }
    }
}

void WindowMenuPreheat::screenEvent(window_t *sender, GUI_event_t event, void *param) {
    switch (event) {

    case GUI_event_t::TOUCH_SWIPE_LEFT:
    case GUI_event_t::TOUCH_SWIPE_RIGHT:
        if (index_mapping.is_item_enabled<Item::return_>()) {
            marlin_client::FSM_response(PhasesPreheat::UserTempSelection, Response::Abort);
            return;
        }
        break;

    default:
        break;
    }

    WindowMenuVirtual::screenEvent(sender, event, param);
}

// * ScreenPreheat
ScreenPreheat::ScreenPreheat()
    : ScreenFSM(nullptr, {})
    , menu(this, GuiDefaults::RectScreenNoHeader)
    , header(this) //
{
    CaptureNormalWindow(menu);
}

void preheat_menu::ScreenPreheat::create_frame() {
    // The FSM has a single screen, no need to do anything
}

void preheat_menu::ScreenPreheat::destroy_frame() {
    // The FSM has a single screen, no need to do anything
}

void preheat_menu::ScreenPreheat::update_frame() {
    const PreheatData data = PreheatData::deserialize(fsm_base_data.GetData());
    const auto title = [&] -> const char * {
        switch (data.mode) {
        case PreheatMode::None:
            return N_("Preheating");

        case PreheatMode::Load:
        case PreheatMode::Autoload:
        case PreheatMode::Change_phase2: // use load caption, not a bug
            return N_("Preheating for load");

        case PreheatMode::Unload:
        case PreheatMode::Change_phase1: // use unload caption, not a bug
            return N_("Preheating for unload");

        case PreheatMode::Purge:
            return N_("Preheating for purge");

        default:
            return N_("---");
        }
    }();

    header.SetText(_(title));
    menu.menu.set_data(data);
}
