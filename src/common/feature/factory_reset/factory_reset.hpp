#pragma once

#include <bitset>

#include <utils/enum_array.hpp>

class FactoryReset {

public:
    using ItemFlag = config_store_ns::ItemFlag;

    /// Specific items that can be kept or reset
    enum class Item {
        calibrations,
        common_misconfigurations,
        network,
        user_interface,
        stats,
        hw_config,
        features,
        printer_state,
        user_profiles,

        _cnt
    };

    using ItemBitset = std::bitset<std::to_underlying(Item::_cnt)>;

    struct ItemConfig {
        const char *title;

        /// What config store items are related to this item
        journal::ItemFlags item_flags;
    };

    static constexpr EnumArray<Item, FactoryReset::ItemConfig, FactoryReset::Item::_cnt> items_config {
        { Item::calibrations, { N_("Calibrations"), ItemFlag::calibrations } },
        { Item::common_misconfigurations, { N_("Common Misconfigurations"), ItemFlag::common_misconfigurations | ItemFlag::dev_items } },
        { Item::network, { N_("Network Settings"), ItemFlag::network } },
        { Item::user_interface, { N_("UI Settings"), ItemFlag::user_interface } },
        { Item::stats, { N_("Statistics"), ItemFlag::stats } },
        { Item::hw_config, { N_("HW Configuration"), ItemFlag::hw_config } },
        { Item::features, { N_("Printer Functions"), ItemFlag::features } },
        { Item::printer_state, { N_("Printer State"), ItemFlag::printer_state } },
        { Item::user_profiles, { N_("User Presets"), ItemFlag::user_presets } },
    };

public:
    /// Performs a factory reset with the specified parameters.
    /// !!! Has to be executed on the GUI thread
    [[noreturn]] static void perform(bool hard_reset, ItemBitset items_to_keep);

public:
    /// Constructs ItemBitset with the specified items set
    static constexpr ItemBitset item_bitset(const std::initializer_list<Item> &lst) {
        ItemBitset result;
        for (auto i : lst) {
            result.set(std::to_underlying(i));
        }
        return result;
    }

    /// Constructs ItemBitset with all EXCEPT the specified items set
    static constexpr ItemBitset item_bitset_exclude(const std::initializer_list<Item> &lst) {
        ItemBitset result(static_cast<uint32_t>(-1));
        static_assert(std::to_underlying(Item::_cnt) <= 32);
        for (auto i : lst) {
            result.reset(std::to_underlying(i));
        }
        return result;
    }
};
