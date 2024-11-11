#include "factory_reset.hpp"

#include <w25x.h>
#include <wdt.hpp>
#include <sys.h>

#include <Marlin.h>

#include <window_msgbox.hpp>
#include <ScreenHandler.hpp>
#include <bootloader/bootloader.hpp>
#include <freertos/critical_section.hpp>
#include <config_store/store_definition.hpp>
#include <option/bootloader.h>

// Check that our presets cover all the item flags
static_assert([] {
    journal::ItemFlags store_flags = 0;
    config_store_ns::CurrentStore s;
    visit_all_struct_fields(s, [&](auto &item) {
        store_flags |= item.flags;
    });

    journal::ItemFlags preset_flags = 0;
    for (const auto &item : FactoryReset::items_config) {
        preset_flags |= item.item_flags;
    }

    return preset_flags == store_flags;
}());

extern osThreadId displayTaskHandle;

[[noreturn]] void FactoryReset::perform(bool hard_reset, ItemBitset items_to_keep) {
    assert(osThreadGetId() == displayTaskHandle);

    MsgBoxBuilder {
        .type = MsgBoxType::pepa_centered,
        .text = _("Performing factory reset.\nPlease wait..."),
        .responses = Responses_NONE,
        .loop_callback = [] {
            // Close the dialog immediately (but it will keep being displayed because we dont't redraw the screen)
            Screens::Access()->Close();
        },
    }
        .exec();

    // Disable task switching, we don't want anyone to screw up with anything now
    freertos::CriticalSection critical_section;

    // Kick the watchdog
    wdt_iwdg_refresh();

    // Disable all sorts of stuff
    {
        thermalManager.disable_all_heaters();
        thermalManager.zero_fan_speeds();
        disable_all_steppers();
    }

    // Wipe EEPROM
    for (uint16_t address = 0; address <= (8096 - 4); address += 4) {
        static constexpr uint32_t empty = ~0;
        st25dv64k_user_write_bytes(address, &empty, 4);
        wdt_iwdg_refresh();
    }

    if (hard_reset) {
        // Wipe xFlash
        w25x_chip_erase();

#if BOOTLOADER()
        // Invalidate firmware by erasing part of it
        if (!buddy::bootloader::fw_invalidate()) {
            bsod("Error invalidating firmware");
        }
#endif

    } else if (items_to_keep.any()) {
        // Initialize a blank config store and save there our selection of items.
        // Values of these items are being kept in the RAM.

        config_store().init();

        // Do not do default config_sotre().load_all(), it would overwrite our items in the RAM.
        // Instead we provide a stub loader and migrators, so we only end up initializing the EEPROM structure properly.
        config_store().get_backend().load_all([](const auto &, const auto &) {}, {});

        assert(config_store().get_backend().get_journal_state() == journal::Backend::JournalState::ColdStart);

        journal::ItemFlags exclude_flags;
        for (size_t i = 0; i < std::to_underlying(Item::_cnt); i++) {
            if (!items_to_keep.test(i)) {
                exclude_flags |= items_config[i].item_flags;
            }
        }
        config_store().dump_items(exclude_flags);
    }

    // We cannot display msg box here - it would be trying to load an icon from xFlash that we've probably wiped.
    // Also, fonts might be in xFlash one day.
    // Just restart the system
    sys_reset();
}
