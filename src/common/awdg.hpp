#pragma once

#include <inplace_function.hpp>
#include <memory>
#include <span>

namespace awdg {

class ADCWatchdogDeleter;

/// Every HW can have different implementation for watchdog. This class should provide basic common interface over it.
class ADCWatchdog {
public:
    /// Callback function type
    using Callback = stdext::inplace_function<void(ADCWatchdog &)>;
    /// Save RAII wrapper - calls reset automatically on destruction
    using Handle = std::unique_ptr<ADCWatchdog, ADCWatchdogDeleter>;
    using ChannelIndex = uint8_t;
    /// Changes the callback function
    virtual void change_callback(Callback callback) = 0;
    /// Reconfigures the watchdogs range
    virtual void adjust_range(uint16_t low, uint16_t high) = 0;
    /// Turns off the watchdog, if blocking, then releases the mutex
    virtual void release() = 0;
};

/// Helper deleter, that resets adc watchdog on destruction of ADCWatchdog::Handler
class ADCWatchdogDeleter {
public:
    void operator()(ADCWatchdog *awdg) {
        awdg->release();
    }
};

/// Gets HW specific implementation of watchdog, blocks if watchdog is in use.
[[nodiscard]] ADCWatchdog::Handle get_single_channel_watchdog(void *adc, ADCWatchdog::ChannelIndex channel, ADCWatchdog::Callback &&callback, uint16_t low, uint16_t high);
/// Gets HW specific implementation of watchdog, blocks if watchdog is in use.
[[nodiscard]] ADCWatchdog::Handle get_multi_channel_watchdog(void *adc, std::span<ADCWatchdog::ChannelIndex> channels, ADCWatchdog::Callback &&callback, uint16_t low, uint16_t high);
/// Gets HW specific implementation of watchdog, blocks if watchdog is in use.
/// Only watches configured channels
[[nodiscard]] ADCWatchdog::Handle get_all_channel_analog_watchdog(void *adc, ADCWatchdog::Callback &&callback, uint16_t low, uint16_t high);

} // namespace awdg
