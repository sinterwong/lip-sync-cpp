#ifndef _BASIC_CORE_LOGGER_LOGGER_HPP_
#define _BASIC_CORE_LOGGER_LOGGER_HPP_

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE

// You can define SPDLOG_ACTIVE_LEVEL to the desired log level before including
// "spdlog.h". This will turn on/off logging statements at compile time
#include <spdlog/spdlog.h>

// logger setting
#define LOGGER_NAME "basic"
#define LOGGER_LOGGER_ERROR_FILENAME "logs/basic_error.log"
#define LOGGER_LOGGER_TRACE_FILENAME "logs/basic_error.log"
#define LOGGER_PATTERN "[%Y-%m-%d %H:%M:%S.%e][%^%l%$][%t][%s:%#] %v"
#define LOGGER_ROTATING_MAX_FILE_SIZE (1024 * 1024)
#define LOGGER_ROTATING_MAX_FILE_NUM 5

#define _TRACE 0
#define _DEBUG 1
#define _INFO 2
#define _WARN 3
#define _ERROR 4
#define _CRITI 5
#define _OFF 6

#define LOGGER_TRACE(...)                                                      \
  SPDLOG_LOGGER_TRACE(spdlog::get(LOGGER_NAME), __VA_ARGS__)
#define LOGGER_DEBUG(...)                                                      \
  SPDLOG_LOGGER_DEBUG(spdlog::get(LOGGER_NAME), __VA_ARGS__)
#define LOGGER_INFO(...)                                                       \
  SPDLOG_LOGGER_INFO(spdlog::get(LOGGER_NAME), __VA_ARGS__)
#define LOGGER_WARN(...)                                                       \
  SPDLOG_LOGGER_WARN(spdlog::get(LOGGER_NAME), __VA_ARGS__)
#define LOGGER_ERROR(...)                                                      \
  SPDLOG_LOGGER_ERROR(spdlog::get(LOGGER_NAME), __VA_ARGS__)
#define LOGGER_CRITICAL(...)                                                   \
  SPDLOG_LOGGER_CRITICAL(spdlog::get(LOGGER_NAME), __VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif

void LipSyncLoggerInit(const bool with_color_console, const bool with_console,
                       const bool with_error, const bool with_trace);

void LipSyncLoggerSetLevel(const int level);

void LipSyncLoggerSetPattern(const char *format);

void LipSyncLoggerSetFlushEvery(const int interval);

void LipSyncLoggerDrop();

#ifdef __cplusplus
}
#endif
#endif