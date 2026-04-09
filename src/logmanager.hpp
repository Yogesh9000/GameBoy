#pragma once

// Pull in SPDLOG_ACTIVE_LEVEL before any spdlog header sees it.
// CMake injects SPDLOG_ACTIVE_LEVEL via compile definitions.
#include <spdlog/fmt/ostr.h>  // enables logging of types with operator
#include <spdlog/spdlog.h>

#include <memory>
#include <string>

namespace LogManager
{

/// level_str: "TRACE","DEBUG","INFO","WARN","ERROR","OFF"
/// log_file:  path to file sink, or "" to disable file logging
void InitLogging(const std::string& level_str = GB_LOG_LEVEL,
    const std::string& log_file = "");

/// Get (or create) a named module logger.
/// Use one logger per translation unit / subsystem.
/// Example: auto log = myproject::get_logger("cpu");
std::shared_ptr<spdlog::logger> GetLogger(const std::string& name);

/// Shut down spdlog cleanly (flush all sinks). Call before exit.
void ShutdownLogging();

}  // namespace LogManager

// ── Convenience macros ────────────────────────────────────────────────────────
// These wrap SPDLOG_LOGGER_* so the call site passes its module logger.
// Usage: LOG_DEBUG(logger, "value = {}", x);
#define LOG_TRACE(logger, ...)    SPDLOG_LOGGER_TRACE(logger, __VA_ARGS__)
#define LOG_DEBUG(logger, ...)    SPDLOG_LOGGER_DEBUG(logger, __VA_ARGS__)
#define LOG_INFO(logger, ...)     SPDLOG_LOGGER_INFO(logger, __VA_ARGS__)
#define LOG_WARN(logger, ...)     SPDLOG_LOGGER_WARN(logger, __VA_ARGS__)
#define LOG_ERROR(logger, ...)    SPDLOG_LOGGER_ERROR(logger, __VA_ARGS__)
#define LOG_CRITICAL(logger, ...) SPDLOG_LOGGER_CRITICAL(logger, __VA_ARGS__)
