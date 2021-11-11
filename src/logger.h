
#ifndef SRC_LOGGER
#define SRC_LOGGER

#include <stdio.h>
#include <stdlib.h>

#include <log4cxx/helpers/exception.h>
#include <log4cxx/helpers/pool.h>
#include <log4cxx/logger.h>
#include <log4cxx/patternlayout.h>
#include <log4cxx/propertyconfigurator.h>

#include <map>
#include <string>
#include <type_traits>
#include <utility>

#define SET_CONFIGURATION(configuration) \
  log4cxx::PropertyConfigurator::configure(configuration);

#define DECLARE_LOGGER() static log4cxx::LoggerPtr logger;

#define DEFINE_LOGGER(namespace, logName) \
  log4cxx::LoggerPtr namespace ::logger = log4cxx::Logger::getLogger(logName);

#define ELOG_MAX_BUFFER_SIZE 10000

#define SPRINTF_ELOG_MSG(buffer, fmt, args...) \
  char buffer[ELOG_MAX_BUFFER_SIZE];           \
  snprintf(buffer, ELOG_MAX_BUFFER_SIZE, fmt, ##args);

#define ELOG_TRACE2(logger, fmt, args...) \
  SPRINTF_ELOG_MSG(__tmp, fmt, ##args);   \
  LOG4CXX_TRACE(logger, __tmp);

#define ELOG_DEBUG2(logger, fmt, args...) \
  SPRINTF_ELOG_MSG(__tmp, fmt, ##args);   \
  LOG4CXX_DEBUG(logger, __tmp);

#define ELOG_INFO2(logger, fmt, args...) \
  SPRINTF_ELOG_MSG(__tmp, fmt, ##args);  \
  LOG4CXX_INFO(logger, __tmp);

#define ELOG_WARN2(logger, fmt, args...) \
  SPRINTF_ELOG_MSG(__tmp, fmt, ##args);  \
  LOG4CXX_WARN(logger, __tmp);

#define ELOG_ERROR2(logger, fmt, args...) \
  SPRINTF_ELOG_MSG(__tmp, fmt, ##args);   \
  LOG4CXX_ERROR(logger, __tmp);

#define ELOG_FATAL2(logger, fmt, args...) \
  SPRINTF_ELOG_MSG(__tmp, fmt, ##args);   \
  LOG4CXX_FATAL(logger, __tmp);

namespace detail {
// Helper for forwarding correctly the object to be logged
template <typename T>
struct LogElementForwarder {
  T operator()(T t) { return t; }
};

template <>
struct LogElementForwarder<std::string> {
  template <typename S>
  const char* operator()(const S& t) {
    return t.c_str();
  }
};
}  // namespace detail

#define DEFINE_ELOG_T(name, invoke)                                            \
  template <typename Logger, typename... Args>                                 \
  inline void name(const Logger&, const char*, Args...)                        \
      __attribute__((always_inline));                                          \
                                                                               \
  template <typename Logger, typename... Args>                                 \
  void name(const Logger& logger, const char* fmt, Args... args) {             \
    invoke(logger, fmt,                                                        \
           detail::LogElementForwarder<typename std::decay<Args>::type>{}(     \
               args)...);                                                      \
  }                                                                            \
                                                                               \
  template <typename Logger>                                                   \
  inline void name(const Logger&, const char*) __attribute__((always_inline)); \
                                                                               \
  template <typename Logger>                                                   \
  void name(const Logger& logger, const char* fmt) {                           \
    invoke(logger, "%s", fmt);                                                 \
  }

DEFINE_ELOG_T(ELOG_TRACET, ELOG_TRACE2)
DEFINE_ELOG_T(ELOG_DEBUGT, ELOG_DEBUG2)
DEFINE_ELOG_T(ELOG_INFOT, ELOG_INFO2)
DEFINE_ELOG_T(ELOG_WARNT, ELOG_WARN2)
DEFINE_ELOG_T(ELOG_ERRORT, ELOG_ERROR2)
DEFINE_ELOG_T(ELOG_FATALT, ELOG_FATAL2)

// older versions of log4cxx don't support tracing
#ifdef LOG4CXX_TRACE
#define ELOG_TRACE(fmt, args...)      \
  if (logger->isTraceEnabled()) {     \
    ELOG_TRACET(logger, fmt, ##args); \
  }
#else
#define ELOG_TRACE(fmt, args...)      \
  if (logger->isDebugEnabled()) {     \
    ELOG_DEBUGT(logger, fmt, ##args); \
  }
#endif

#define ELOG_DEBUG(fmt, args...)      \
  if (logger->isDebugEnabled()) {     \
    ELOG_DEBUGT(logger, fmt, ##args); \
  }

#define ELOG_INFO(fmt, args...)      \
  if (logger->isInfoEnabled()) {     \
    ELOG_INFOT(logger, fmt, ##args); \
  }

#define ELOG_WARN(fmt, args...)      \
  if (logger->isWarnEnabled()) {     \
    ELOG_WARNT(logger, fmt, ##args); \
  }

#define ELOG_ERROR(fmt, args...)      \
  if (logger->isErrorEnabled()) {     \
    ELOG_ERRORT(logger, fmt, ##args); \
  }

#define ELOG_FATAL(fmt, args...)      \
  if (logger->isFatalEnabled()) {     \
    ELOG_FATALT(logger, fmt, ##args); \
  }

#endif /* SRC_LOGGER */
