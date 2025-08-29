#pragma once
#include <format>
#include <iostream>
#include <string>

// logging
inline void Info(const std::string &msg)
{
  std::clog << std::format("\033[32m[info]\033[0m {}", msg);
}

inline void Warning(const std::string &msg)
{
  std::clog << std::format("\033[33m[warn]\033[0m {}", msg);
}

inline void Error(const std::string &msg)
{
  std::clog << std::format("\033[31m[error]\033[0m {}", msg);
}
