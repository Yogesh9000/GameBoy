find_package(nlohmann_json CONFIG REQUIRED)

add_executable(cpu_test cpu_test.cpp)
target_link_libraries(cpu_test PRIVATE GTest::gtest GTest::gtest_main
                                  nlohmann_json::nlohmann_json)

include(GoogleTest)
gtest_discover_tests(cpu_test WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/tests/single_step_tests/data)
