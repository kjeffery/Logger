#pragma once

#include <atomic>
#include <cassert>
#include <format>
#include <iostream>
#include <source_location>
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

inline AtomicLoggingInformation g_logging_information{
    { LogLevel::WARNING, false }
};

constexpr std::string_view to_string(LogLevel level) noexcept
{
    using enum LogLevel;
    switch (level) {
        case DEBUG:
            return "Debug";
        case INFO:
            return "Info";
        case WARNING:
            return "Warning";
        case ERROR:
            return "Error";
        case FATAL:
            return "Fatal";
    }

    assert(!"Should not get here");
    std::unreachable();
}
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

namespace detail {
template <typename... Args>
void do_log(std::ostream&               outs,
            LogLevel                    level,
            std::source_location        location,
            std::format_string<Args...> fmt,
            Args&&...                   args)
{
    const auto info = get_logging_state();
    if (level != LogLevel::FATAL && !is_enabled_for_level(level, info.m_level)) {
        return;
    }

    const auto       level_string = detail::to_string(level);
    std::osyncstream synced_out{ outs };
    if (level == LogLevel::DEBUG || info.m_verbose) {
        if (location.function_name()) {
            std::println(synced_out,
                         "{}: [{}:{} ({})]: {}",
                         level_string,
                         location.file_name(),
                         location.line(),
                         location.function_name(),
                         std::format(fmt, std::forward<Args>(args)...));
        } else {
            std::println(synced_out,
                         "{}: [{}:{}]: {}",
                         level_string,
                         location.file_name(),
                         location.line(),
                         std::format(fmt, std::forward<Args>(args)...));
        }
    } else {
        std::println(synced_out, "{}: {}", level_string, std::format(fmt, std::forward<Args>(args)...));
    }
}
} // namespace detail

template <typename... Args>
struct log_debug
{
    log_debug(std::format_string<Args...> fmt,
              Args&&...                   args,
              std::source_location        location = std::source_location::current())
    {
        detail::do_log(std::clog, LogLevel::DEBUG, location, fmt, std::forward<Args>(args)...);
    }
};

template <typename... Args>
struct log_info
{
    log_info(std::format_string<Args...> fmt,
             Args&&...                   args,
             std::source_location        location = std::source_location::current())
    {
        detail::do_log(std::clog, LogLevel::INFO, location, fmt, std::forward<Args>(args)...);
    }
};

template <typename... Args>
struct log_warning
{
    log_warning(std::format_string<Args...> fmt,
                Args&&...                   args,
                std::source_location        location = std::source_location::current())
    {
        detail::do_log(std::cerr, LogLevel::WARNING, location, fmt, std::forward<Args>(args)...);
    }
};

template <typename... Args>
struct log_error
{
    log_error(std::format_string<Args...> fmt,
              Args&&...                   args,
              std::source_location        location = std::source_location::current())
    {
        detail::do_log(std::cerr, LogLevel::ERROR, location, fmt, std::forward<Args>(args)...);
    }
};

template <typename... Args>
struct log_fatal
{
    log_fatal(std::format_string<Args...> fmt,
              Args&&...                   args,
              std::source_location        location = std::source_location::current())
    {
        detail::do_log(std::cerr, LogLevel::FATAL, location, fmt, std::forward<Args>(args)...);
        std::exit(EXIT_FAILURE);
    }
};

// I use user-defined deduction guides because I have conflicting desires:
// 1. I want to allow variadic templates for arbitrary formatting
// 2. I want to use std::source_location, which requires the use of a default argument
//
// The user-defined deduction guides allow me to pass on the variadic templates while deferring the defaulted
// std::source_location instantiation to the constructor.
template <typename... Args>
log_debug(std::format_string<Args...> fmt, Args&&... args) -> log_debug<Args...>;

template <typename... Args>
log_info(std::format_string<Args...> fmt, Args&&... args) -> log_info<Args...>;

template <typename... Args>
log_warning(std::format_string<Args...> fmt, Args&&... args) -> log_warning<Args...>;

template <typename... Args>
log_error(std::format_string<Args...> fmt, Args&&... args) -> log_error<Args...>;

template <typename... Args>
log_fatal(std::format_string<Args...> fmt, Args&&... args) -> log_fatal<Args...>;
} // namespace logging
