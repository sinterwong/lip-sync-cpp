#include "logger/logger.hpp"

int main() {
  LipSyncLoggerInit(true, true, true, true);
  LOGGER_TRACE("hello logger, {}", 2020);
  LOGGER_DEBUG("hello logger, {}", 2020);
  LOGGER_INFO("hello logger, {}", 2020);
  LOGGER_WARN("hello logger, {}", 2020);
  LOGGER_ERROR("hello logger, {}", 2020);
  LOGGER_CRITICAL("hello logger, {}", 2020);
  LipSyncLoggerDrop();
  return 0;
}
