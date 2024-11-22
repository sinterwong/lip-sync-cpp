#ifndef _BASIC_CORE_LOGGER_LOGGER_HPP_
#define _BASIC_CORE_LOGGER_LOGGER_HPP_

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE

// You can define SPDLOG_ACTIVE_LEVEL to the desired log level before including
// "spdlog.h". This will turn on/off logging statements at compile time
#include <spdlog/spdlog.h>

// logger setting
#define BASIC_LOGGER_NAME "basic"
#define BASIC_LOGGER_LOGGER_ERROR_FILENAME "logs/basic_error.log"
#define BASIC_LOGGER_LOGGER_TRACE_FILENAME "logs/basic_error.log"
#define BASIC_LOGGER_PATTERN "[%Y-%m-%d %H:%M:%S.%e][%^%l%$][%t][%s:%#] %v"
#define BASIC_LOGGER_ROTATING_MAX_FILE_SIZE (1024 * 1024)
#define BASIC_LOGGER_ROTATING_MAX_FILE_NUM 5

#define _TRACE 0
#define _DEBUG 1
#define _INFO 2
#define _WARN 3
#define _ERROR 4
#define _CRITI 5
#define _OFF 6

#define BASIC_LOGGER_TRACE(...)                                                \
  LipSyncLoggerOut(_TRACE, __FILE__, __LINE__, ##__VA_ARGS__)
#define BASIC_LOGGER_DEBUG(...)                                                \
  LipSyncLoggerOut(_DEBUG, __FILE__, __LINE__, ##__VA_ARGS__)
#define BASIC_LOGGER_INFO(...)                                                 \
  LipSyncLoggerOut(_INFO, __FILE__, __LINE__, ##__VA_ARGS__)
#define BASIC_LOGGER_WARN(...)                                                 \
  LipSyncLoggerOut(_WARN, __FILE__, __LINE__, ##__VA_ARGS__)
#define BASIC_LOGGER_ERROR(...)                                                \
  LipSyncLoggerOut(_ERROR, __FILE__, __LINE__, ##__VA_ARGS__)
#define BASIC_LOGGER_CRITICAL(...)                                             \
  LipSyncLoggerOut(_CRITI, __FILE__, __LINE__, ##__VA_ARGS__)

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

template <typename... T>
void LipSyncLoggerOut(const int level, const char *filename, const int line,
                      T &&...args) {
  // Note: sdplog::get is a thread safe function
  std::shared_ptr<spdlog::logger> logger_ptr = spdlog::get(BASIC_LOGGER_NAME);
  if (!logger_ptr) {
    fprintf(stderr, "Failed to get logger, Please init logger firstly.\n");
    return; // Add this to prevent potential null pointer dereference
  }
#ifdef _MSC_VER
  // MSVC
  logger_ptr->info("{}, {}, {}", filename, line, SPDLOG_FUNCTION);
  logger_ptr->log(static_cast<spdlog::level::level_enum>(level), "Message: ");
#else
  // GCC
  logger_ptr->log(spdlog::source_loc{filename, line, SPDLOG_FUNCTION},
                  static_cast<spdlog::level::level_enum>(level),
                  std::forward<T>(args)...);
#endif
}

#endif
