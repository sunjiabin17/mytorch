cmake_minimum_required(VERSION 3.20 FATAL_ERROR)
project(c10 CXX)

set(CMAKE_CXX_STANDARD 17 CACHE STRING "The C++ standard whose features are requested.")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB C10_SRCS
        "*.cpp"
        "util/*.cpp"
        "core/*.cpp"
        "core/impl/*.cpp"
        "cpu/*.cpp"
        "cpu/impl/*.cpp"
    )

file(GLOB C10_HEADERS
        "*.h"
        "util/*.h"
        "core/*.h"
        "core/impl/*.h"
        "cpu/*.h"
        "cpu/impl/*.h"
    )

add_library(c10 SHARED ${C10_SRCS} ${C10_HEADERS})

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DESTINATION include
        FILES_MATCHING PATTERN "*.h")
