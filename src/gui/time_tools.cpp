#include "time_tools.hpp"
#include <config_store/store_instance.hpp>

namespace time_tools {

TimeFormat get_time_format() {
    return config_store().time_format.get();
}

void set_timezone_minutes_offset(TimezoneOffsetMinutes new_offset) {
    config_store().timezone_minutes.set(new_offset);
}

TimezoneOffsetMinutes get_timezone_minutes_offset() {
    return config_store().timezone_minutes.get();
}

void set_timezone_summertime_offset(TimezoneOffsetSummerTime new_offset) {
    config_store().timezone_summer.set(new_offset);
}

TimezoneOffsetSummerTime get_timezone_summertime_offset() {
    return config_store().timezone_summer.get();
}

int32_t calculate_total_timezone_offset_minutes() {
    const int8_t timezone = config_store().timezone.get();
    const TimezoneOffsetMinutes timezone_minutes = config_store().timezone_minutes.get();

    static_assert(ftrstd::to_underlying(TimezoneOffsetSummerTime::no_summertime) == 0);
    static_assert(ftrstd::to_underlying(TimezoneOffsetSummerTime::summertime) == 1);

    return //
        static_cast<int32_t>(timezone) * 60

        // Minutes timezone, through lookup table, clamped for memory safety
        + ((timezone < 0) ? -1 : 1) * timezone_offset_minutes_value[std::min(static_cast<size_t>(timezone_minutes), timezone_offset_minutes_value.size() - 1)] //

        // Summer/wintertime. Summertime = +1, so we can simply do a static cast
        + static_cast<int32_t>(config_store().timezone_summer.get()) * 60;
}

namespace {
    struct CachedTime {
        uint8_t hour = 0;
        uint8_t minute = 0;
        TimeFormat format = TimeFormat::_cnt; // Invalid value for initial update force

        auto operator<=>(const CachedTime &) const = default;
    };

    char text_buffer[] = "--:-- --"; ///< Buffer for time string, needs to fit "01:23 am"
    CachedTime cached_time;
} // namespace

time_t get_local_time() {
    time_t t = time(nullptr);

    // Check if time is initialized in RTC (from sNTP)
    if (t == invalid_time) {
        return invalid_time;
    }

    t += calculate_total_timezone_offset_minutes() * 60;

    return t;
}

bool update_time() {
    CachedTime new_time {
        .format = config_store().time_format.get()
    };

    time_t t = get_local_time();
    struct tm now;
    if (t != invalid_time) {
        localtime_r(&t, &now);
        new_time.hour = now.tm_hour;
        new_time.minute = now.tm_min;
    }

    if (new_time == cached_time) {
        return false;
    }

    cached_time = new_time;

    if (t == invalid_time) {
        const char *fmt = cached_time.format == TimeFormat::_12h ? "--:-- --" : "--:--";
        strlcpy(text_buffer, fmt, sizeof(text_buffer));
    } else {
        const char *format_str = (cached_time.format == TimeFormat::_24h) ? "%H:%M" : "%I:%M %p";
        strftime(text_buffer, std::size(text_buffer), format_str, &now);
    }

    // Time changed and was printed
    return true;
}

const char *get_time() {
    return text_buffer;
}

} // namespace time_tools
