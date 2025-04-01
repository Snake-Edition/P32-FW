#include "screen_m600.hpp"
#include <general_response.hpp>
#include <window_msgbox.hpp>
#include <ScreenHandler.hpp>
#include <WindowMenuItems.hpp>
#include <screen_menu.hpp>
#include <img_resources.hpp>
#include <timing.h>

namespace {

bool enqueued = false; // Used to avoid multiple M600 enqueue

bool confirm_fil_change() {
    return (MsgBoxQuestion(_("Perform filament change now?"), Responses_YesNo) == Response::Yes);
};

bool inject(const uint8_t tool) {
    if (!confirm_fil_change()) {
        return false;
    }
    marlin_client::inject(GCodeLiteral("M600 P T%.0f", static_cast<float>(tool)));
    enqueued = true;
    return true;
};

#if HAS_TOOLCHANGER()
// for available toolheads
class MI_M600_SUBMENU : public IWindowMenuItem {
public:
    MI_M600_SUBMENU(const uint8_t tool)
        : IWindowMenuItem({}, nullptr, is_enabled_t::yes,
            prusa_toolchanger.is_tool_enabled(tool) ? is_hidden_t::no : is_hidden_t::yes)
        , tool(tool) {
        StringBuilder sb(label_buffer_);
        sb.append_string_view(_("Tool"));
        sb.append_printf(" %d", tool + 1);
        SetLabel(string_view_utf8::MakeRAM(label_buffer_.data()));
    }

    void click([[maybe_unused]] IWindowMenu &window_menu) override {
        if (inject(tool)) {
            Screens::Access()->Close();
        }
    }

private:
    std::array<char, 32> label_buffer_;
    const uint8_t tool;
};

class MI_M600_SUBMENU_ACTIVE : public IWindowMenuItem {
public:
    MI_M600_SUBMENU_ACTIVE()
        : IWindowMenuItem(_("Active Tool"), nullptr, is_enabled_t::yes, is_hidden_t::no) {}

    void click([[maybe_unused]] IWindowMenu &window_menu) override {
        if (inject(marlin_vars().active_extruder)) {
            Screens::Access()->Close();
        }
    }
};
template <typename I>
struct MenuBase_;

template <size_t... i>
struct MenuBase_<std::index_sequence<i...>> {
    using T = ScreenMenu<GuiDefaults::MenuFooter, MI_RETURN,
        MI_M600_SUBMENU_ACTIVE,
        WithConstructorArgs<MI_M600_SUBMENU, i>...>;
};
using MenuBase = MenuBase_<std::make_index_sequence<HOTENDS>>::T;

class M600_SUBMENU final : public MenuBase {
public:
    M600_SUBMENU()
        : MenuBase(_("CHANGE FILAMENT")) {};
};
#endif
} // namespace

MI_M600::MI_M600()
    : IWindowMenuItem(_(label), nullptr, is_enabled_t::yes, is_hidden_t::no,
#if HAS_TOOLCHANGER()
        (prusa_toolchanger.is_toolchanger_enabled()) ? expands_t::yes :
#endif
                                                     expands_t::no) {
    set_icon_position(IconPosition::right);
}

void MI_M600::click([[maybe_unused]] IWindowMenu &window_menu) {
#if HAS_TOOLCHANGER()
    if (prusa_toolchanger.is_toolchanger_enabled()) {
        Screens::Access()->Open(ScreenFactory::Screen<M600_SUBMENU>);
        return;
    }
#endif
    inject(marlin_vars().active_extruder);
}

void MI_M600::Loop() {
    update_enqueued_icon();
    handle_enable_state();
}

void MI_M600::update_enqueued_icon() {
    if (enqueued) {
        const auto current_ms = ticks_ms();
        SetIconId(img::spinner_16x16_stages[(current_ms / 256) % img::spinner_16x16_stages.size()]);
    } else {
        SetIconId(nullptr);
    }
}

void MI_M600::handle_enable_state() {
    const auto current_command = marlin_vars().gcode_command.get();

    if (current_command == marlin_server::Cmd::M600) {
        enqueued = false;
    }
    // M600 during printing is enabled the moment after first layer started printing
    // M600 is incompatible with initializing gcodes such as G29 a G28
    set_enabled(!enqueued && (marlin_vars().max_printed_z > 0));
}
