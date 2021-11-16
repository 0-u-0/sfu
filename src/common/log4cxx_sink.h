#ifndef SRC_LOG4CXX_SINK_H_
#define SRC_LOG4CXX_SINK_H_

#include "logger.h"
#include "rtc_base/logging.h"

// Log sink that uses a FileRotatingStream to write to disk.
// Init() must be called before adding this sink.
class Log4cxxSink : public rtc::LogSink {
 public:
  DECLARE_LOGGER()
  Log4cxxSink();
  ~Log4cxxSink() override;

  // Writes the message to the current file. It will spill over to the next
  // file if needed.
  void OnLogMessage(const std::string& message) override;
  void OnLogMessage(const std::string& message,
                    rtc::LoggingSeverity sev) override;

 private:
  RTC_DISALLOW_COPY_AND_ASSIGN(Log4cxxSink);
};

#endif /* SRC_LOG4CXX_SINK_H_ */
