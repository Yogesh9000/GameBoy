add_executable(${PROJECT_NAME}
                   "main.cpp"
                   "cpu.hpp"
                   "cpu.cpp"
                   "bootrom.hpp"
                   "bootrom.cpp"
                   "concretememoryrange.hpp"
                   "concretememoryrange.cpp"
                   "sdldisplay.hpp"
                   "sdldisplay.cpp"
                   "ppu.hpp"
                   "ppu.cpp"
                   "mmu.hpp"
                   "mmu.cpp"
                   "filememoryrange.hpp"
                   "filememoryrange.cpp"
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
