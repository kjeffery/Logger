#pragma once

#include <atomic>
#include <format>
#include <iostream>
#include <type_traits>

namespace logging {

enum class LogLevel
{
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    FATAL
};

namespace detail {
using LevelBaseType = std::underlying_type_t<LogLevel>;

std::atomic<LogLevel> g_log_level{ LogLevel::WARNING };
} // namespace detail

// Returns previous level
inline LogLevel set_logging_level(LogLevel level) noexcept
{
    return detail::g_log_level.exchange(level);
}

inline LogLevel get_logging_level() noexcept
{
    return detail::g_log_level.load();
}

inline bool enabled_for_level(LogLevel level) noexcept
{
    const auto current_level     = get_logging_level();
    const auto level_int         = static_cast<detail::LevelBaseType>(level);
    const auto current_level_int = static_cast<detail::LevelBaseType>(current_level);

    return current_level_int <= level_int;
}

template <typename... Args>
void log_debug(const char* const file_name, int line, std::format_string<Args...> fmt, Args&&... args)
{
    if (enabled_for_level(LogLevel::DEBUG)) {
        std::println(std::cout, "Debug [{}:{}]: {}", file_name, line, std::format(fmt, std::forward<Args>(args)...));
    }
}

template <typename... Args>
void log_info(std::format_string<Args...> fmt, Args&&... args)
{
    if (enabled_for_level(LogLevel::INFO)) {
        std::println(std::cout, "Info: {}", std::format(fmt, std::forward<Args>(args)...));
    }
}

template <typename... Args>
void log_warning(std::format_string<Args...> fmt, Args&&... args)
{
    if (enabled_for_level(LogLevel::WARNING)) {
        std::println(std::cerr, "Warning: {}", std::format(fmt, std::forward<Args>(args)...));
    }
}

template <typename... Args>
void log_error(std::format_string<Args...> fmt, Args&&... args)
{
    if (enabled_for_level(LogLevel::ERROR)) {
        std::println(std::cerr, "Error: {}", std::format(fmt, std::forward<Args>(args)...));
    }
}

template <typename... Args>
void log_fatal(std::format_string<Args...> fmt, Args&&... args)
{
    std::println(std::cerr, "Fatal: {}", std::format(fmt, std::forward<Args>(args)...));
    std::exit(EXIT_FAILURE);
}

} // namespace logging

#define LOG_DEBUG(fmt, ...)   logging::log_debug(__FILE__, __LINE__, fmt __VA_OPT__(, ) __VA_ARGS__)
#define LOG_INFO(fmt, ...)    logging::log_info(fmt __VA_OPT__(, ) __VA_ARGS__)
#define LOG_WARNING(fmt, ...) logging::log_warning(fmt __VA_OPT__(, ) __VA_ARGS__)
#define LOG_ERROR(fmt, ...)   logging::log_error(fmt __VA_OPT__(, ) __VA_ARGS__)
#define LOG_FATAL(fmt, ...)   logging::log_fatal(fmt __VA_OPT__(, ) __VA_ARGS__)
