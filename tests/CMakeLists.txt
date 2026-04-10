find_package(GTest CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_executable(cpu_test test_main.cpp "cpu_test_single_step_test.cpp"
                                     "${PROJECT_SOURCE_DIR}/src/cpu.cpp"
                                     "${PROJECT_SOURCE_DIR}/src/mmu.cpp"
                                     "${PROJECT_SOURCE_DIR}/src/interrupt.cpp"
                                     "${PROJECT_SOURCE_DIR}/src/logmanager.cpp"
                                     "${PROJECT_SOURCE_DIR}/src/concretememoryrange.cpp")

target_include_directories(cpu_test PRIVATE "${PROJECT_SOURCE_DIR}/src")
target_link_libraries(cpu_test PRIVATE GTest::gtest
                                  nlohmann_json::nlohmann_json
                                  spdlog::spdlog)

apply_logging_settings(cpu_test)

include(GoogleTest)
gtest_discover_tests(cpu_test WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/tests/data)
