
#include "logger.h"

// using namespace log4cxx;

namespace log4cxx {
IMPLEMENT_LOG4CXX_OBJECT(ColorPatternLayout);
ColorPatternLayout::ColorPatternLayout() : PatternLayout() {}

ColorPatternLayout::ColorPatternLayout(const LogString& s) : PatternLayout(s) {}

void ColorPatternLayout::format(LogString& output,
                                const spi::LoggingEventPtr& event,
                                helpers::Pool& pool) const {
  LogString tmp;
  PatternLayout::format(tmp, event, pool);
  LevelPtr lvl = event->getLevel();
  switch (lvl->toInt()) {
    case Level::FATAL_INT:
      output.append("\u001b[0;41m");  // red BG
      break;
    case Level::ERROR_INT:
      output.append("\u001b[0;31m");  // red FG
      break;
    case Level::WARN_INT:
      output.append("\u001b[0;33m");  // Yellow FG
      break;
    case Level::INFO_INT:
      output.append("\u001b[32m");  // Bright
      break;
    case Level::DEBUG_INT:
      output.append("\u001b[36m");  // Green FG
      break;
    case Level::TRACE_INT:
      output.append("\u001b[0;30m");  // Black FG
      break;
    default:
      break;
  }
  output.append(tmp);
  output.append("\u001b[m");
}

}  // namespace log4cxx
