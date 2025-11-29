#pragma once

#include <utils/string_builder.hpp>

#include "print_status_message.hpp"

class PrintStatusMessageFormatterBuddy {

public:
    using Message = PrintStatusMessage;
    using Params = StringViewUtf8Parameters<16>;

    static void format(StringBuilder &target, const Message &msg);
};
