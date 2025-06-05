#include "factory_reset.hpp"

#include <common/w25x.hpp>
#include <wdt.hpp>
#include <common/st25dv64k.h>
#include <common/sys.hpp>

#include <Marlin.h>

#include <window_msgbox.hpp>
#include <ScreenHandler.hpp>
#include <bootloader/bootloader.hpp>
#include <freertos/critical_section.hpp>
#include <config_store/store_definition.hpp>
#include <option/bootloader.h>
#include <gui.hpp>
#include <sound.hpp>

#include <option/has_phase_stepping.h>
#if HAS_PHASE_STEPPING()
    #include <feature/phase_stepping/phase_stepping.hpp>
#endif

// Check that our presets cover all the item flags
static constexpr auto store_flags = [] {
    journal::ItemFlags store_flags = 0;
    config_store_ns::CurrentStore s {};
    visit_all_struct_fields(s, [&](auto &item) {
        store_flags |= item.flags;
    });

    // Exclude special items, they are special
    store_flags &= ~config_store_ns::ItemFlag::special;

    return store_flags;
}();
static constexpr auto preset_flags = [] {
    journal::ItemFlags preset_flags = 0;
    for (const auto &item : FactoryReset::items_config) {
        preset_flags |= item.item_flags;
    }

    return preset_flags;
}();
static_assert(store_flags == preset_flags);

extern osThreadId displayTaskHandle;

[[noreturn]] void FactoryReset::perform(bool hard_reset, ItemBitset items_to_keep) {
    assert(osThreadGetId() == displayTaskHandle);

    MsgBoxBuilder {
        .type = MsgBoxType::warning,
        .text = _("Performing factory reset.\nDo not turn off the printer."),
        .responses = Responses_NONE,
        .loop_callback = [&] {
            // Force screen update; it might not always happen in gui_loop thanks to timers
            Screens::Access()->Draw();

            // Close the dialog immediately (but it will keep being displayed because we dont't redraw the screen)
            Screens::Access()->Close();
        },
    }
        .exec();

    // Stop any sound plays. We will be entering a critical section and don't want to hear a long beep during all that wiping
    Sound_Stop();
    Sound_Update1ms();

#if HAS_PHASE_STEPPING()
    // Phase stepping is a calibration that is stored on xFlash, not in the config store -> it needs special handling
    // On hard reset, we're clearing the whole xFlash anyway, no point in doing this separately
    if (!hard_reset && !items_to_keep.test(std::to_underlying(Item::calibrations))) {
        // Do this before entering the critical section, as it does all sorts of file access and logging

        // Formally speaking, we should access phase_stepping only from the marlin thread.
        // But all this function does is unlink files from the xFlash.
        // These files are only accessed in specific phstep gcodes
        // and it's not worth ensuring that they would happen to overwrite before we enter the final factory reset critical section a few lines below
        phase_stepping::remove_from_persistent_storage();
    }
#endif

    // Disable task switching, we don't want anyone to screw up with anything now
    freertos::CriticalSection critical_section;

    // freertos::CriticalSection actually only disables preemptive context switches,
    // so if we happen to get stuck on some mutex (I2C/config store/flash),
    // we would lose the context and things we don't want might happen
    // To remedy this, let's give this thread the highest priority
    // so that the OS returns here as soon as a mutex is unlocked
    osThreadSetPriority(osThreadGetId(), osPriorityRealtime);

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
        w25x_chip_erase([] {
            wdt_iwdg_refresh();
        });

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

        journal::ItemFlags exclude_flags = 0;
        for (size_t i = 0; i < std::to_underlying(Item::_cnt); i++) {
            if (!items_to_keep.test(i)) {
                exclude_flags |= items_config[i].item_flags;
            }
        }
        config_store().dump_items(exclude_flags);

        // If we're resetting hw config, make sure that we perform first run config setup
        if (exclude_flags & config_store_ns::ItemFlag::hw_config) {
            config_store().force_default_hw_config.set(true);
        }

        config_store().config_version.set(config_store_ns::CurrentStore::newest_config_version);
    }

    // We cannot display msg box here - it would be trying to load an icon from xFlash that we've probably wiped.
    // Also, fonts might be in xFlash one day.
    // Just restart the system
    sys_reset();
}
