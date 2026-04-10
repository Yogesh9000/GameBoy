#include "logmanager.hpp"

#include <spdlog/details/registry.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <vector>

namespace LogManager
{

static spdlog::level::level_enum ParseLevel(const std::string& s)
{
  if (s == "TRACE") return spdlog::level::trace;
  if (s == "DEBUG") return spdlog::level::debug;
  if (s == "INFO") return spdlog::level::info;
  if (s == "WARN") return spdlog::level::warn;
  if (s == "ERROR") return spdlog::level::err;
  if (s == "CRITICAL") return spdlog::level::critical;
  if (s == "OFF") return spdlog::level::off;
  return spdlog::level::info;  // safe default
}

void InitLogging(const std::string& level_str, const std::string& log_file)
{
  auto level = ParseLevel(level_str);

  // Build the sink list
  std::vector<spdlog::sink_ptr> sinks;

  auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  console_sink->set_level(level);
  sinks.push_back(console_sink);

  if (!log_file.empty())
  {
    auto file_sink =
        std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_file, true);
    file_sink->set_level(spdlog::level::trace);  // file always gets everything
    sinks.push_back(file_sink);
  }

  // Create a default logger with these sinks
  auto default_logger =
      std::make_shared<spdlog::logger>("default", sinks.begin(), sinks.end());
  default_logger->set_level(level);
  spdlog::set_default_logger(default_logger);

  // IMPORTANT: also set the global level so runtime matches compile-time gate
  spdlog::set_level(level);

  spdlog::flush_on(spdlog::level::warn);  // auto-flush on warnings+
}

std::shared_ptr<spdlog::logger> GetLogger(const std::string& name)
{
  // Return existing logger if already registered
  auto existing = spdlog::get(name);
  if (existing) return existing;

  // Clone sinks from the default logger
  auto default_logger = spdlog::default_logger();
  auto logger = std::make_shared<spdlog::logger>(
      name, default_logger->sinks().begin(), default_logger->sinks().end());
  logger->set_level(default_logger->level());
  spdlog::register_logger(logger);
  return logger;
}

void ShutdownLogging()
{
  spdlog::shutdown();
}

}  // namespace LogManager
