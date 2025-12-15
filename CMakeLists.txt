cmake_minimum_required(VERSION 3.31.6)

project(NoobBoy VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SDL3 CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

add_executable(${PROJECT_NAME} src/main.cpp
                   "src/bootrom.hpp"
                   "src/bootrom.cpp"
                   "src/concretememoryrange.hpp"
                   "src/concretememoryrange.cpp"
                   "src/sdldisplay.hpp"
                   "src/sdldisplay.cpp"
                   "src/ppu.hpp"
                   "src/ppu.cpp"
                   "src/mmu.hpp"
                   "src/mmu.cpp"
                   "src/filememoryrange.hpp"
                   "src/filememoryrange.cpp"
)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog)
target_compile_options(${PROJECT_NAME} PRIVATE
    # Common warnings for GCC and Clang
    $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-Wall -Wextra -Wconversion -Wsign-conversion>

    # Common warnings for MSVC
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG>
    $<$<CONFIG:Release>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_ERROR>
)
