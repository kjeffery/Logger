#include "Logging.h"

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
    std::clog << "=========================\n";
    std::println(std::clog, "Testing with level: {} and verbosity {}",
                 static_cast<int>(logging::get_logging_level_state()),
                 logging::get_logging_verbose_state());

    int x = 42;
    logging::log_debug("Test 1");
    logging::log_debug("Test {}", 2);
    logging::log_debug("Test {} with a value of {}", 3, x);

    logging::log_info("Test 4");
    logging::log_info("Test {}", 5);

    logging::log_warning("Test 6");
    logging::log_warning("Test {} test", 7);

    logging::log_warning("Test 8: {}", f());
    logging::log_warning("Test 9: {}", f(2, 3));

    logging::log_error("Test 10");
    logging::log_error("Test {} test", 11);
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

    logging::log_fatal("Test {}", 42);
}
