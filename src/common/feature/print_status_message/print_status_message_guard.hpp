#pragma once

#include "print_status_message.hpp"

/// Shows and keeps a status message for the guard lifetime
/// !!! To be called only from the marlin thread - we need to keep the stack order
class PrintStatusMessageGuard {
    friend class PrintStatusMessageManager;

public:
    using Message = PrintStatusMessage;
    using Record = PrintStatusMessageRecord;

    explicit PrintStatusMessageGuard();
    explicit PrintStatusMessageGuard(const Message &msg);
    PrintStatusMessageGuard(const PrintStatusMessageGuard &) = delete;

    ~PrintStatusMessageGuard();

    const Record &record() const {
        return record_;
    }

    /// Changes the message of the guard. Does not change message id.
    void update(const Message &msg);

    template <Message::Type type>
    void update(const Message::TypeRecordOf<type>::Data &data) {
        update(Message::make<type>(data));
    }

private:
    Record record_;
    PrintStatusMessageGuard *parent_guard_;
};
