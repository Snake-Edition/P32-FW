#include "print_status_message_mgr.hpp"

#include <timing.h>

PrintStatusMessageManager print_status_message_instance;

PrintStatusMessageManager &print_status_message() {
    return print_status_message_instance;
}

PrintStatusMessageManager::Record PrintStatusMessageManager::current_message() const {
    std::scoped_lock mutex_guard(mutex_);

    if (temporary_message_.data) {
        return temporary_message_.data;
    }

    auto guard = active_guard_;
    while (guard) {
        const auto &data = guard->record();
        if (data.message.type != PrintStatusMessage::none) {
            return data;
        }

        guard = guard->parent_guard_;
    }

    return {};
}

void PrintStatusMessageManager::show_temporary(const Message &msg, uint32_t duration_ms) {
    std::scoped_lock guard(mutex_);
    temporary_message_ = TemporaryMessage {
        .data {
            .message = msg,
            .id = id_counter_++,
        },
        .end_time_ms = ticks_ms() + duration_ms,
    };
    add_history_item_nolock(temporary_message_.data);
}

void PrintStatusMessageManager::clear_timed_out_temporary() {
    std::scoped_lock guard(mutex_);
    if (temporary_message_.data && ticks_diff(ticks_ms(), temporary_message_.end_time_ms) >= 0) {
        temporary_message_.data = {};
    }
}

void PrintStatusMessageManager::clear_temporary() {
    std::scoped_lock guard(mutex_);
    temporary_message_ = {};
}

void PrintStatusMessageManager::walk_history(const stdext::inplace_function<bool(const Record &)> &callback) const {
    std::scoped_lock guard(mutex_);
    const auto end = history_pos_;

    auto pos = history_pos_;
    do {
        pos = (pos + 1) % history_buffer_size;
        const Record &msg = history_[pos];
        if (msg) {
            callback(msg);
        }
    } while (pos != end);
}

void PrintStatusMessageManager::add_history_item_nolock(const Record &msg) {
    // If the message ID is the same, update history, otherwise advance it
    if (history_[history_pos_].id != msg.id) {
        history_pos_ = (history_pos_ + 1) % history_buffer_size;
    }

    history_[history_pos_] = msg;
}
