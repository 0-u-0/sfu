
#include "log4cxx_sink.h"

#include <iostream>
#include "logger.h"

#include "absl/strings/strip.h"

DEFINE_LOGGER(Log4cxxSink, "Log4cxxSink");

Log4cxxSink::Log4cxxSink() {}

Log4cxxSink::~Log4cxxSink() {}

void Log4cxxSink::OnLogMessage(const std::string& message) {
  // nothing to do
}

void Log4cxxSink::OnLogMessage(const std::string& message,
                               rtc::LoggingSeverity sev) {
  std::string mes = message;
  absl::StripTrailingAsciiWhitespace(&mes);
  if (sev == rtc::LoggingSeverity::LS_VERBOSE) {
    DLOG(mes);
  } else if (sev == rtc::LoggingSeverity::LS_INFO) {
    ILOG(mes);
  } else if (sev == rtc::LoggingSeverity::LS_WARNING) {
    WLOG(mes);
  } else if (sev == rtc::LoggingSeverity::LS_ERROR) {
    ELOG(mes);
  }
}
