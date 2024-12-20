#pragma once

#include <str_utils.hpp>

#include "print_status_message.hpp"

class PrintStatusMessageFormatterBuddy {

public:
    using Message = PrintStatusMessage;
    using Params = StringViewUtf8Parameters<16>;

    static void format(StringBuilder &target, const Message &msg);
};
