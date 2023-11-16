#pragma once

#include <atomic>
#include <format>
#include <iostream>
#include <syncstream>
#include <type_traits>

namespace logging {

enum class LogLevel : std::uint8_t
{
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    FATAL
};

// Bundling these values makes the atomic code a little uglier than having separate atomic values, but results in fewer
// atomic operation in general usage.
struct LoggingInformation
{
    LogLevel m_level;
    bool     m_verbose;
};

namespace detail {
using LevelBaseType = std::underlying_type_t<LogLevel>;

using AtomicLoggingInformation = std::atomic<LoggingInformation>;
static_assert(AtomicLoggingInformation::is_always_lock_free, "This isn't strictly necessary, but I want to know");

AtomicLoggingInformation g_logging_information{
    { LogLevel::WARNING, false }
};
} // namespace detail

// Returns previous level
inline LogLevel set_logging_level_state(LogLevel level) noexcept
{
    LoggingInformation expected = detail::g_logging_information.load();
    LoggingInformation desired{ level, expected.m_verbose };

    while (!detail::g_logging_information.compare_exchange_weak(expected, desired)) {
        // Somebody may have updated verbosity while we weren't looking.
        desired.m_verbose = expected.m_verbose;
    }
    return expected.m_level;
}

inline LogLevel get_logging_level_state() noexcept
{
    return detail::g_logging_information.load().m_level;
}

inline bool set_logging_verbose_state(bool b) noexcept
{
    LoggingInformation expected = detail::g_logging_information.load();
    LoggingInformation desired{ expected.m_level, b };

    while (!detail::g_logging_information.compare_exchange_weak(expected, desired)) {
        // Somebody may have updated the level while we weren't looking.
        desired.m_level = expected.m_level;
    }
    return expected.m_verbose;
}

inline bool get_logging_verbose_state() noexcept
{
    return detail::g_logging_information.load().m_verbose;
}

inline LoggingInformation set_logging_state(LogLevel level, bool verbose) noexcept
{
    return detail::g_logging_information.exchange({ level, verbose });
}

inline LoggingInformation get_logging_state() noexcept
{
    return detail::g_logging_information.load();
}

inline bool is_enabled_for_level(LogLevel level, LogLevel global_level) noexcept
{
    const auto level_int        = static_cast<detail::LevelBaseType>(level);
    const auto global_level_int = static_cast<detail::LevelBaseType>(global_level);

    return global_level_int <= level_int;
}

inline bool is_enabled_for_level(LogLevel level) noexcept
{
    const auto global_level = get_logging_level_state();
    return is_enabled_for_level(level, global_level);
}

template <typename... Args>
void log_debug(const char* const file_name, int line, std::format_string<Args...> fmt, Args&&... args)
{
    const auto info = get_logging_state();
    if (!is_enabled_for_level(LogLevel::DEBUG, info.m_level)) {
        return;
    }
    std::osyncstream synced_out{ std::cout };
    std::println(synced_out, "Debug [{}:{}]: {}", file_name, line, std::format(fmt, std::forward<Args>(args)...));
}

template <typename... Args>
void log_info(const char* const file_name, int line, std::format_string<Args...> fmt, Args&&... args)
{
    const auto info = get_logging_state();
    if (!is_enabled_for_level(LogLevel::INFO, info.m_level)) {
        return;
    }
    std::osyncstream synced_out{ std::cout };
    if (info.m_verbose) {
        std::println(synced_out, "Info: [{}:{}]: {}", file_name, line, std::format(fmt, std::forward<Args>(args)...));
    } else {
        std::println(synced_out, "Info: {}", std::format(fmt, std::forward<Args>(args)...));
    }
}

template <typename... Args>
void log_warning(const char* const file_name, int line, std::format_string<Args...> fmt, Args&&... args)
{
    const auto info = get_logging_state();
    if (!is_enabled_for_level(LogLevel::WARNING, info.m_level)) {
        return;
    }
    std::osyncstream synced_out{ std::cerr };
    if (info.m_verbose) {
        std::println(synced_out,
                     "Warning: [{}:{}]: {}",
                     file_name,
                     line,
                     std::format(fmt, std::forward<Args>(args)...));
    } else {
        std::println(synced_out, "Warning: {}", std::format(fmt, std::forward<Args>(args)...));
    }
}

template <typename... Args>
void log_error(const char* const file_name, int line, std::format_string<Args...> fmt, Args&&... args)
{
    const auto info = get_logging_state();
    if (!is_enabled_for_level(LogLevel::ERROR, info.m_level)) {
        return;
    }
    std::osyncstream synced_out{ std::cerr };
    if (info.m_verbose) {
        std::println(synced_out, "Error: [{}:{}]: {}", file_name, line, std::format(fmt, std::forward<Args>(args)...));
    } else {
        std::println(synced_out, "Error: {}", std::format(fmt, std::forward<Args>(args)...));
    }
}

template <typename... Args>
void log_fatal(const char* const file_name, int line, std::format_string<Args...> fmt, Args&&... args)
{
    const auto       info = get_logging_state();
    std::osyncstream synced_out{ std::cerr };
    if (info.m_verbose) {
        std::println(synced_out, "Fatal: [{}:{}]: {}", file_name, line, std::format(fmt, std::forward<Args>(args)...));
    } else {
        std::println(synced_out, "Fatal: {}", std::format(fmt, std::forward<Args>(args)...));
    }
    std::exit(EXIT_FAILURE);
}

} // namespace logging

#define LOG_DEBUG(fmt, ...)   logging::log_debug(__FILE__, __LINE__, fmt __VA_OPT__(, ) __VA_ARGS__)
#define LOG_INFO(fmt, ...)    logging::log_info(__FILE__, __LINE__, fmt __VA_OPT__(, ) __VA_ARGS__)
#define LOG_WARNING(fmt, ...) logging::log_warning(__FILE__, __LINE__, fmt __VA_OPT__(, ) __VA_ARGS__)
#define LOG_ERROR(fmt, ...)   logging::log_error(__FILE__, __LINE__, fmt __VA_OPT__(, ) __VA_ARGS__)
#define LOG_FATAL(fmt, ...)   logging::log_fatal(__FILE__, __LINE__, fmt __VA_OPT__(, ) __VA_ARGS__)
