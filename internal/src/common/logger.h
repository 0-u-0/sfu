
#ifndef SRC_LOGGER
#define SRC_LOGGER

#include <fmt/format.h>
#include <log4cxx/logger.h>
#include <log4cxx/propertyconfigurator.h>

extern log4cxx::LoggerPtr globalLogger;

#define LOG_CONFIGURATION(configuration) \
  log4cxx::PropertyConfigurator::configure(configuration);

#define DECLARE_LOGGER() static log4cxx::LoggerPtr logger;

#define DEFINE_LOGGER(namespace, logName) \
  log4cxx::LoggerPtr namespace ::logger = log4cxx::Logger::getLogger(logName);

// #define LOG_TRACE(fmt, args...) \
//   LOG4CXX_TRACE_FMT(logger, fmt, ##args);

// #define LOG4CXX_DEBUG_FMT(logger, ...)                               \
//   do {                                                               \
//     if (LOG4CXX_UNLIKELY(logger->isDebugEnabled())) {                \
//       logger->forcedLog(::log4cxx::Level::getDebug(),                \
//                         fmt::format(__VA_ARGS__), LOG4CXX_LOCATION); \
//     }                                                                \
//   } while (0)

// #define LOG4CXX_TRACE_FMT(logger, ...)                               \
//   do {                                                               \
//     if (LOG4CXX_UNLIKELY(logger->isTraceEnabled())) {                \
//       logger->forcedLog(::log4cxx::Level::getTrace(),                \
//                         fmt::format(__VA_ARGS__), LOG4CXX_LOCATION); \
//     }                                                                \
//   } while (0)

// #define LOG4CXX_INFO_FMT(logger, ...)                                          \
//   do {                                                                         \
//     if (logger->isInfoEnabled()) {                                             \
//       logger->forcedLog(::log4cxx::Level::getInfo(), fmt::format(__VA_ARGS__), \
//                         LOG4CXX_LOCATION);                                     \
//     }                                                                          \
//   } while (0)

// #define LOG4CXX_WARN_FMT(logger, ...)                                          \
//   do {                                                                         \
//     if (logger->isWarnEnabled()) {                                             \
//       logger->forcedLog(::log4cxx::Level::getWarn(), fmt::format(__VA_ARGS__), \
//                         LOG4CXX_LOCATION);                                     \
//     }                                                                          \
//   } while (0)

// #define LOG4CXX_ERROR_FMT(logger, ...)                               \
//   do {                                                               \
//     if (logger->isErrorEnabled()) {                                  \
//       logger->forcedLog(::log4cxx::Level::getError(),                \
//                         fmt::format(__VA_ARGS__), LOG4CXX_LOCATION); \
//     }                                                                \
//   } while (0)

#define DLOG(...) LOG4CXX_DEBUG_FMT(logger, __VA_ARGS__);
#define ILOG(...) LOG4CXX_INFO_FMT(logger, __VA_ARGS__);
#define WLOG(...) LOG4CXX_WARN_FMT(logger, __VA_ARGS__);
#define ELOG(...) LOG4CXX_ERROR_FMT(logger, __VA_ARGS__);

#define DGLOG(...) LOG4CXX_DEBUG_FMT(globalLogger, __VA_ARGS__);
#define IGLOG(...) LOG4CXX_INFO_FMT(globalLogger, __VA_ARGS__);
#define WGLOG(...) LOG4CXX_WARN_FMT(globalLogger, __VA_ARGS__);
#define EGLOG(...) LOG4CXX_ERROR_FMT(globalLogger, __VA_ARGS__);

// #define LOG_ERROR(fmt, args...) LOG4CXX_ERROR_FMT(logger, fmt, ##args);

// #define LOG_FATAL(fmt, args...) LOG4CXX_FATAL_FMT(logger, fmt, ##args);

#endif /* SRC_LOGGER */
