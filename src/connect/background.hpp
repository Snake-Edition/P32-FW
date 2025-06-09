#pragma once

#include "command.hpp"

#include <common/shared_buffer.hpp>
#include <feature/cork/tracker.hpp>

#include <variant>

namespace connect_client {

class Printer;

struct BackgroundGcodeContent {
    // Stored without \0 at the back.
    SharedBorrow data;
    size_t size;
    size_t position;
};

struct BackgroundGcodeWait {
    buddy::cork::Tracker::CorkHandle cork;
    bool submitted;
};

using BackgroundGcode = std::variant<BackgroundGcodeContent, BackgroundGcodeWait>;

enum class BackgroundResult {
    Success,
    Failure,
    More,
    Later,
};

using BackgroundCmd = std::variant<BackgroundGcode>;

BackgroundResult background_cmd_step(BackgroundCmd &cmd, Printer &printer);

} // namespace connect_client
