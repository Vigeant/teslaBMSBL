#include "TimeLib.h"
/*
   Logger.cpp

  Copyright (c) 2013 Collin Kidder, Michael Neuweiler, Charles Galpin

  Permission is hereby granted, free of charge, to any person obtaining
  a copy of this software and associated documentation files (the
  "Software"), to deal in the Software without restriction, including
  without limitation the rights to use, copy, modify, merge, publish,
  distribute, sublicense, and/or sell copies of the Software, and to
  permit persons to whom the Software is furnished to do so, subject to
  the following conditions:

  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include "Logger.hpp"

//instantiate the logger
Logger log_inst;

/////////////////////////////////////////////////
/// \brief Constructor for the logger
/////////////////////////////////////////////////
Logger::Logger() {
  //logLevel = Logger::Info;
  logLevel = Logger::Cons;
  lastLogTime = 0;
}

/////////////////////////////////////////////////
/// \brief Output a debug message on the console
/// with a variable amount of parameters printf() style
/////////////////////////////////////////////////
void Logger::debug(const char *message, ...) {
  if (logLevel > Debug)
    return;
  va_list args;
  va_start(args, message);
  log(Debug, message, args);
  va_end(args);
}

/////////////////////////////////////////////////
/// \brief Output a info message on the console
/// with a variable amount of parameters printf() style
/////////////////////////////////////////////////
void Logger::info(const char *message, ...) {
  if (logLevel > Info)
    return;
  va_list args;
  va_start(args, message);
  log(Info, message, args);
  va_end(args);
}

/////////////////////////////////////////////////
/// \brief Output a warning message on the console
/// with a variable amount of parameters printf() style
/////////////////////////////////////////////////
void Logger::warn(const char *message, ...) {
  if (logLevel > Warn)
    return;
  va_list args;
  va_start(args, message);
  log(Warn, message, args);
  va_end(args);
}

/////////////////////////////////////////////////
/// \brief Output a error message on the console
/// with a variable amount of parameters printf() style
/////////////////////////////////////////////////
void Logger::error(const char *message, ...) {
  if (logLevel > Error)
    return;
  va_list args;
  va_start(args, message);
  log(Error, message, args);
  va_end(args);
}

/////////////////////////////////////////////////
/// \brief Output a console message on the console
/// with a variable amount of parameters printf() style
/////////////////////////////////////////////////
void Logger::console(const char *message, ...) {
  //return;

  va_list args;
  va_start(args, message);
  log(Cons, message, args);

  va_end(args);
}

/////////////////////////////////////////////////
/// \brief Set the log level. Any output below the specified log level will be omitted.
///
/// @param level (0 = debug, 1=info, 2=warning, 3=error, 4=supress all)
/////////////////////////////////////////////////
void Logger::setLoglevel(LogLevel level) {
  logLevel = level;
}

/////////////////////////////////////////////////
/// \brief Retrieve the current log level.
/////////////////////////////////////////////////
Logger::LogLevel Logger::getLogLevel() {
  return logLevel;
}

/////////////////////////////////////////////////
/// \brief Return a timestamp when the last log entry was made.
/////////////////////////////////////////////////
uint32_t Logger::getLastLogTime() {
  return lastLogTime;
}

/////////////////////////////////////////////////
/// \brief Returns if debug log level is enabled.
///
/// This can be used in time critical
/// situations to prevent unnecessary string concatenation (if the message won't
/// be logged in the end).
///
/// Example:
///
/// if (Logger::isDebug()) {
///
///    Logger::debug("current time: %d", millis());
///
/// }
/////////////////////////////////////////////////
bool Logger::isDebug() {
  return logLevel == Debug;
}

/////////////////////////////////////////////////
/// \brief prints timestamp.
///
/// This can be used to print a timestamp
///
/////////////////////////////////////////////////
void Logger::printTimeStamp(time_t t) {
  tmElements_t tm;
  breakTime(t, tm);
  //console("%04d-%02d-%02dT%02d:%02d:%02d", tm.Year + 1970, tm.Month, tm.Day, tm.Hour, tm.Minute, tm.Second);
  console("%s %02d, %02d %02d:%02d:%02d", monthShortStr(tm.Month), tm.Day, tm.Year + 1970, tm.Hour, tm.Minute, tm.Second);
}

/////////////////////////////////////////////////
/// \brief prints timestamp.
///
/// This can be used to print a timestamp
///
/////////////////////////////////////////////////
void Logger::printTimeStampLn(time_t t) {
  printTimeStamp(t);
  console("\n");
}

/////////////////////////////////////////////////
/// \brief Outputs a message to screen
/////////////////////////////////////////////////
void Logger::log(LogLevel level, const char *format, va_list args) {
  if (level < Cons) {
    lastLogTime = millis();
    SERIALCONSOLE.print(lastLogTime);
    SERIALCONSOLE.print(" - ");
  }
  switch (level) {
    case Debug:
      SERIALCONSOLE.print("DEBUG  :");
      break;
    case Info:
      SERIALCONSOLE.print("INFO   :");
      break;
    case Warn:
      SERIALCONSOLE.print("WARNING:");
      break;
    case Error:
      SERIALCONSOLE.print("ERROR  :");
      break;
    case Off:
    case Cons:
      break;
  }
  logMessage(format, args);
}

/////////////////////////////////////////////////
/// \brief Outputs a message to screen
/////////////////////////////////////////////////
void Logger::logMessage(const char *format, va_list args) {
  char buf[128];  // resulting string limited to 128 chars
  vsnprintf(buf, 128, format, args);
  SERIALCONSOLE.print(buf);
}