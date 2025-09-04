cmake_minimum_required(VERSION 3.31.6)

project(NoobBoy VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_compile_options(-Wconversion)

find_package(SDL3 CONFIG REQUIRED)

add_executable(main src/main.cpp
                    src/bootrom.cpp
                    src/concretememoryrange.cpp
                    src/sdldisplay.cpp
                    src/ppu.cpp
                    src/mmu.cpp
)
target_link_libraries(main PRIVATE SDL3::SDL3)
