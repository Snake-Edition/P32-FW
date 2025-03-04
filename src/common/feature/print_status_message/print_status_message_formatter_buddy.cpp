#include "print_status_message_formatter_buddy.hpp"

#include <buddy/unreachable.hpp>

using Message = PrintStatusMessage;

static constexpr EnumArray<Message::Type, const char *, Message::Type::_cnt> messages {
    { Message::Type::none, nullptr },
        { Message::Type::custom, nullptr },
        { Message::Type::homing, N_("Homing") },
        { Message::Type::recalibrating_home, N_("Recalibrating home. This may take some time.") },
        { Message::Type::calibrating_axis, N_("Calibrating axis") },
        { Message::Type::probing_bed, N_("Probing bed") },
        { Message::Type::additional_probing, N_("Additional probing") },
        { Message::Type::dwelling, N_("Dwelling") },
        { Message::Type::absorbing_heat, N_("Absorbing heat") },
        { Message::Type::waiting_for_hotend_temp, N_("Waiting for hotend") },
        { Message::Type::waiting_for_bed_temp, N_("Waiting for bed") },
#if ENABLED(PRUSA_SPOOL_JOIN)
        { Message::Type::spool_joined, N_("Spool joined") },
        { Message::Type::joining_spool, N_("Joining spool") },
#endif
#if HAS_CHAMBER_API()
        { Message::Type::waiting_for_chamber_temp, N_("Waiting for chamber") },
#endif
};

void PrintStatusMessageFormatterBuddy::format(StringBuilder &target, const Message &msg) {
    if (auto txt = messages.get_or(msg.type, nullptr)) {
        target.append_string_view(_(txt));
    }

    switch (msg.type) {

    case Message::Type::custom: {
        const auto d = std::get<PrintStatusMessageDataCustom>(msg.data);
        target.append_string(d.message.get());
        break;
    }

    case Message::Type::calibrating_axis: {
        const auto d = std::get<PrintStatusMessageDataAxisProgress>(msg.data);
        target.append_printf(" (%c)", axis_codes[d.axis]);
        break;
    }

    case Message::Type::probing_bed:
    case Message::Type::additional_probing: {
        const auto d = std::get<PrintStatusMessageDataProgress>(msg.data);
        target.append_printf("\n%i/%i", (int)d.current, (int)d.target);
        break;
    }

    case Message::Type::dwelling:
    case Message::Type::absorbing_heat: {
        const auto d = std::get<PrintStatusMessageDataProgress>(msg.data);
        const int val = (int)d.current;
        target.append_printf("\n%i:%02i", val / 60, val % 60);
        break;
    }

    case Message::Type::waiting_for_hotend_temp:
    case Message::Type::waiting_for_bed_temp:
#if HAS_CHAMBER_API()
    case Message::Type::waiting_for_chamber_temp:
#endif
    {
        const auto d = std::get<PrintStatusMessageDataProgress>(msg.data);
        target.append_printf("\n%i/%i °C", (int)std::round(d.current), (int)std::round(d.target));
        break;
    }

    default:
        break;
    }
}
