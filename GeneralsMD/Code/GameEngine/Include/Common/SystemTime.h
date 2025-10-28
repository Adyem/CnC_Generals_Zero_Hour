
#ifndef COMMON_SYSTEM_TIME_H
#define COMMON_SYSTEM_TIME_H

#include <chrono>
#include <cstdint>
#include <ctime>

struct SystemTime
{
        std::uint16_t year = 0;
        std::uint16_t month = 0;
        std::uint16_t dayOfWeek = 0;
        std::uint16_t day = 0;
        std::uint16_t hour = 0;
        std::uint16_t minute = 0;
        std::uint16_t second = 0;
        std::uint16_t milliseconds = 0;
};

inline std::tm systemTimeToTm(const SystemTime& value)
{
        std::tm result{};
        result.tm_year = (value.year >= 1900)
                ? static_cast<int>(value.year) - 1900
                : static_cast<int>(value.year);
        result.tm_mon = value.month > 0 ? static_cast<int>(value.month) - 1 : 0;
        result.tm_mday = static_cast<int>(value.day);
        result.tm_hour = static_cast<int>(value.hour);
        result.tm_min = static_cast<int>(value.minute);
        result.tm_sec = static_cast<int>(value.second);
        result.tm_wday = static_cast<int>(value.dayOfWeek);
        result.tm_isdst = -1;
        return result;
}

inline SystemTime tmToSystemTime(const std::tm& tmValue, std::uint16_t milliseconds = 0)
{
        SystemTime result{};
        result.year = static_cast<std::uint16_t>(tmValue.tm_year + 1900);
        result.month = static_cast<std::uint16_t>(tmValue.tm_mon + 1);
        result.day = static_cast<std::uint16_t>(tmValue.tm_mday);
        result.dayOfWeek = static_cast<std::uint16_t>(tmValue.tm_wday);
        result.hour = static_cast<std::uint16_t>(tmValue.tm_hour);
        result.minute = static_cast<std::uint16_t>(tmValue.tm_min);
        result.second = static_cast<std::uint16_t>(tmValue.tm_sec);
        result.milliseconds = milliseconds;
        return result;
}

inline SystemTime getCurrentSystemTime()
{
        using clock = std::chrono::system_clock;
        const auto now = clock::now();
        const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % std::chrono::seconds(1);
        const std::time_t timeValue = clock::to_time_t(now);
        std::tm local{};
#if defined(_WIN32)
        localtime_s(&local, &timeValue);
#else
        localtime_r(&timeValue, &local);
#endif
        return tmToSystemTime(local, static_cast<std::uint16_t>(milliseconds.count()));
}

#endif // COMMON_SYSTEM_TIME_H
