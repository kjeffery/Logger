cmake_minimum_required(VERSION 3.27)
project(Logger)

set(CMAKE_CXX_STANDARD 23)

if (MSVC)
    # This is needed on MSVC to enable __VA_OPT__
    add_compile_options("/Zc:preprocessor")
endif()

add_executable(Logger main.cpp
        Logger.h)
