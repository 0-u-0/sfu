
#ifndef SRC_LOGGER
#define SRC_LOGGER

#include <fmt/format.h>
#include <log4cxx/logger.h>
#include <log4cxx/propertyconfigurator.h>

#define LOG_CONFIGURATION(configuration) \
  log4cxx::PropertyConfigurator::configure(configuration);

#define DECLARE_LOGGER() static log4cxx::LoggerPtr logger;

#define DEFINE_LOGGER(namespace, logName) \
  log4cxx::LoggerPtr namespace ::logger = log4cxx::Logger::getLogger(logName);

// #define LOG_TRACE(fmt, args...) \
//   LOG4CXX_TRACE_FMT(logger, fmt, ##args);

#define DLOG(...) LOG4CXX_DEBUG_FMT(logger, __VA_ARGS__);
#define ILOG(...) LOG4CXX_INFO_FMT(logger, __VA_ARGS__);
#define WLOG(...) LOG4CXX_WARN_FMT(logger, __VA_ARGS__);
#define ELOG(...) LOG4CXX_ERROR_FMT(logger, __VA_ARGS__);
// #define LOG_ERROR(fmt, args...) LOG4CXX_ERROR_FMT(logger, fmt, ##args);

// #define LOG_FATAL(fmt, args...) LOG4CXX_FATAL_FMT(logger, fmt, ##args);

#endif /* SRC_LOGGER */
