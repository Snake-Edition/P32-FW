#pragma once

#include <option/has_chamber_api.h>
#include <option/has_auto_retract.h>

#include "print_status_message_data.hpp"

struct PrintStatusMessage {
    enum Type {
        none,

        /// Custom, performatted text
        custom,

        homing,
        homing_retrying,
        homing_refining,
        recalibrating_home,
        calibrating_axis,
        probing_bed,
        additional_probing,

        dwelling,

        absorbing_heat,

        waiting_for_hotend_temp,
        waiting_for_bed_temp,

#if ENABLED(PROBE_CLEANUP_SUPPORT)
        nozzle_cleaning,
#endif
#if ENABLED(PRUSA_SPOOL_JOIN)
        spool_joined,
        joining_spool,
#endif
#if HAS_CHAMBER_API()
        waiting_for_chamber_temp,
#endif
#if HAS_AUTO_RETRACT()
        auto_retracting,
#endif

        _cnt
    };
    using Data = PrintStatusMessageData;

    template <Type type_, typename Data_>
    struct TypeRecord {
        static constexpr Type type = type_;
        using Data = Data_;

        template <typename T>
        consteval auto operator<<(T v) const {
            // We're putting the needle (the type we're looking for) as the rightmost operand in the fold expression
            // So if our type (the left operand) matches type other type, it means that we've got a hit and we should return this record.
            if constexpr (T::type == type) {
                return *this;
            } else {
                return v;
            }
        }
    };

    template <typename... Records_>
    struct TypeRecordList {

        /// \returns TypeRecord() of the type matching type_ or
        template <Type type>
        static consteval auto get_() {
            // This is quite C++ magical, but basically it returns instance of TypeRecord with appropriate for the given type
            constexpr auto needle = TypeRecord<type, void>();
            constexpr auto r = (Records_() << ... << needle);
            static_assert(!std::is_same_v<decltype(r), decltype(needle)>, "Message type missing from the record list");
            return r;
        }
    };

    using type_record_list = TypeRecordList<
        TypeRecord<Type::custom, PrintStatusMessageDataCustom>,
        TypeRecord<Type::homing, std::monostate>,
        TypeRecord<Type::homing_retrying, PrintStatusMessageDataAxisProgress>,
        TypeRecord<Type::homing_refining, PrintStatusMessageDataAxisProgress>,
        TypeRecord<Type::recalibrating_home, std::monostate>,
        TypeRecord<Type::calibrating_axis, PrintStatusMessageDataAxisProgress>,
        TypeRecord<Type::probing_bed, PrintStatusMessageDataProgress>,
        TypeRecord<Type::additional_probing, PrintStatusMessageDataProgress>,
        TypeRecord<Type::dwelling, PrintStatusMessageDataProgress>,
        TypeRecord<Type::absorbing_heat, PrintStatusMessageDataProgress>,
        TypeRecord<Type::waiting_for_hotend_temp, PrintStatusMessageDataProgress>,
        TypeRecord<Type::waiting_for_bed_temp, PrintStatusMessageDataProgress>,
#if ENABLED(PROBE_CLEANUP_SUPPORT)
        TypeRecord<Type::nozzle_cleaning, std::monostate>,
#endif
#if ENABLED(PRUSA_SPOOL_JOIN)
        TypeRecord<Type::spool_joined, std::monostate>,
        TypeRecord<Type::joining_spool, std::monostate>,
#endif
#if HAS_CHAMBER_API()
        TypeRecord<Type::waiting_for_chamber_temp, PrintStatusMessageDataProgress>,
#endif
#if HAS_AUTO_RETRACT()
        TypeRecord<Type::auto_retracting, PrintStatusMessageDataProgress>,
#endif

        TypeRecord<Type::none, std::monostate>>;

    template <Type type>
    using TypeRecordOf = decltype(type_record_list::get_<type>());

    PrintStatusMessage() = default;

    /// This function enforces that the data type matches the one expected from the type_record_list
    template <Type type>
    static PrintStatusMessage make(const TypeRecordOf<type>::Data &data) {
        return PrintStatusMessage(type, data);
    }

    Type type = Type::none;
    Data data;

    bool operator==(const PrintStatusMessage &) const = default;

private:
    /// Private only - use make() which does data type checking
    PrintStatusMessage(Type type, const Data &data)
        : type(type)
        , data(data) {}
};

struct PrintStatusMessageRecord {
    PrintStatusMessage message;
    uint32_t id = 0;

    explicit operator bool() const {
        return message.type != PrintStatusMessage::none;
    }

    bool operator==(const PrintStatusMessageRecord &) const = default;
};
