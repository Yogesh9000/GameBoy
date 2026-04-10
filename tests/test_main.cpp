#include <gtest/gtest.h>

#include "logmanager.hpp"

int main(int argc, char** argv)
{
  LogManager::InitLogging(GB_LOG_LEVEL, "test_log.txt");
  ::testing::InitGoogleTest(&argc, argv);
  auto exit_code = RUN_ALL_TESTS();
  LogManager::ShutdownLogging();
  return exit_code;
}
