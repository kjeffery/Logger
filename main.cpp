#include "Logger.h"

#include <iostream>

int main()
{
    logging::set_logging_level(logging::LogLevel::DEBUG);

    int x = 42;
    LOG_DEBUG("Test 1");
    LOG_DEBUG("Test {}", 2);
    LOG_DEBUG("Test {} with a value of {}", 3, x);

    LOG_INFO("Test 4");
    LOG_INFO("Test {}", 5);

    LOG_WARNING("Test 6");
    LOG_WARNING("Test {} test", 7);

    LOG_ERROR("Test 8");
    LOG_ERROR("Test {} test", 9);

    LOG_FATAL("Test {}", 10);
}
