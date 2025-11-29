#pragma once

#include <freertos/mutex.hpp>
#include <inplace_function.hpp>
#include <common/primitive_any.hpp>

#include <string_view_utf8.hpp>

#include "print_status_message.hpp"
#include "print_status_message_guard.hpp"

class PrintStatusMessageManager {
    friend class PrintStatusMessageGuard;

public:
    using Message = PrintStatusMessage;
    using Guard = PrintStatusMessageGuard;
    using Record = PrintStatusMessageRecord;

    static constexpr uint32_t default_duration_ms = 5000;

    /// How many status messages we want to keep in history
    static constexpr size_t history_buffer_size = 8;

public:
    /// \returns current message to be shown
    Record current_message() const;

    /// Shows a temporary message for the specified duration. Overrides previous temporary message.
    /// To be called only from the marlin thread.
    void show_temporary(const Message &msg, uint32_t duration_ms = default_duration_ms);

    template <Message::Type type>
    void show_temporary(const Message::TypeRecordOf<type>::Data &data, uint32_t duration_ms = default_duration_ms) {
        show_temporary(Message::make<type>(data), duration_ms);
    }

    /// Clears the temporary message
    void clear_temporary();

    /// Clears a temporary message that has already timed out
    void clear_timed_out_temporary();

    /// Walks the history from the oldest item to the newest, calling \param callback for each item
    /// Stops iteration if the callback returns false.
    /// Locks the status message mutex -> be fast!
    void walk_history(const stdext::inplace_function<bool(const Record &)> &callback) const;

private:
    void add_history_item_nolock(const Record &msg);

private:
    mutable freertos::Mutex mutex_;
    Guard *active_guard_ = nullptr;
    uint32_t id_counter_ = 1;

    /// Circular buffer storing recent statuses
    std::array<Record, history_buffer_size> history_;

    /// Position of the newest item in the history
    size_t history_pos_ = 0;

private:
    struct TemporaryMessage {
        Record data;
        uint32_t end_time_ms;
    };
    TemporaryMessage temporary_message_;
};

PrintStatusMessageManager &print_status_message();
