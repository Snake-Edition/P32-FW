#include "print_status_message_guard.hpp"
#include "print_status_message_mgr.hpp"

extern osThreadId defaultTaskHandle;

PrintStatusMessageGuard::PrintStatusMessageGuard() {
    assert(xTaskGetCurrentTaskHandle() == defaultTaskHandle);

    auto &psm = print_status_message();
    std::scoped_lock guard(psm.mutex_);

    parent_guard_ = psm.active_guard_;
    record_.id = psm.id_counter_++;

    // Clear any running temporary message -> gets overriden by the guard
    psm.temporary_message_ = {};
    psm.active_guard_ = this;
}

PrintStatusMessageGuard::PrintStatusMessageGuard(const Message &msg)
    : PrintStatusMessageGuard() {
    update(msg);
}

PrintStatusMessageGuard::~PrintStatusMessageGuard() {
    auto &psm = print_status_message();
    std::scoped_lock guard(psm.mutex_);

    assert(psm.active_guard_ == this);
    psm.active_guard_ = parent_guard_;
}

void PrintStatusMessageGuard::update(const Message &msg) {
    auto &psm = print_status_message();
    std::scoped_lock guard(psm.mutex_);

    record_.message = msg;
    psm.add_history_item_nolock(record_);
}
