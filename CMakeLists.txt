cmake_minimum_required(VERSION 3.31.6)

project(NoobBoy VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

find_package(SDL3 CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(_default_log_level "DEBUG")
else()
    set(_default_log_level "INFO")
endif()

set(LOG_LEVEL "${_default_log_level}" CACHE STRING "Log Level: TRACE DEBUG INFO WARN ERROR OFF")
set_property(CACHE LOG_LEVEL PROPERTY STRINGS TRACE DEBUG INFO WARN ERROR OFF)

message(STATUS "Log level: ${LOG_LEVEL}")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(logging)

add_subdirectory(src)
enable_testing()
add_subdirectory(tests)
