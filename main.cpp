#include "Logger.h"

#include <iostream>

int f()
{
    return 42;
}

int f(int x, int y)
{
    return x * y;
}

void log()
{
    std::cout << "=========================\n";
    std::println(std::cout, "Testing with level: {} and verbosity {}",
                 static_cast<int>(logging::get_logging_level_state()),
                 logging::get_logging_verbose_state());

    int x = 42;
    LOG_DEBUG("Test 1");
    LOG_DEBUG("Test {}", 2);
    LOG_DEBUG("Test {} with a value of {}", 3, x);

    LOG_INFO("Test 4");
    LOG_INFO("Test {}", 5);

    LOG_WARNING("Test 6");
    LOG_WARNING("Test {} test", 7);

    LOG_WARNING("Test 8: {}", f());
    LOG_WARNING("Test 9: {}", f(2, 3));

    LOG_ERROR("Test 10");
    LOG_ERROR("Test {} test", 11);
}

int main()
{
    logging::set_logging_level_state(logging::LogLevel::DEBUG);
    log();
    logging::set_logging_level_state(logging::LogLevel::INFO);
    log();
    logging::set_logging_level_state(logging::LogLevel::WARNING);
    log();
    logging::set_logging_level_state(logging::LogLevel::ERROR);
    log();

    logging::set_logging_verbose_state(true);
    logging::set_logging_level_state(logging::LogLevel::DEBUG);
    log();
    logging::set_logging_level_state(logging::LogLevel::INFO);
    log();
    logging::set_logging_level_state(logging::LogLevel::WARNING);
    log();
    logging::set_logging_level_state(logging::LogLevel::ERROR);
    log();

    LOG_FATAL("Test {}", 42);
}
