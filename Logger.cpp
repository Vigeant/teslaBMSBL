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

Logger::Logger(){
  logLevel = Logger::Info;
  lastLogTime = 0;
}

/*
   Output a debug message with a variable amount of parameters.
   printf() style, see Logger::log()

*/
void Logger::debug(const char *message, ...) {
  if (logLevel > Debug)
    return;
  va_list args;
  va_start(args, message);
  log(Debug, message, args);
  va_end(args);
}

/*
   Output a info message with a variable amount of parameters
   printf() style, see Logger::log()
*/
void Logger::info(const char *message, ...) {
  if (logLevel > Info)
    return;
  va_list args;
  va_start(args, message);
  log(Info, message, args);
  va_end(args);
}

/*
   Output a warning message with a variable amount of parameters
   printf() style, see Logger::log()
*/
void Logger::warn(const char *message, ...) {
  if (logLevel > Warn)
    return;
  va_list args;
  va_start(args, message);
  log(Warn, message, args);
  va_end(args);
}

/*
   Output a error message with a variable amount of parameters
   printf() style, see Logger::log()
*/
void Logger::error(const char *message, ...) {
  if (logLevel > Error)
    return;
  va_list args;
  va_start(args, message);
  log(Error, message, args);
  va_end(args);
}

/*
   Output a comnsole message with a variable amount of parameters
   printf() style, see Logger::logMessage()
*/
void Logger::console(const char *message, ...) {
  //return;
  
  va_list args;
  va_start(args, message);
  log(Cons, message, args);
  
  va_end(args);
}


/*
   Set the log level. Any output below the specified log level will be omitted.
*/
void Logger::setLoglevel(LogLevel level) {
  logLevel = level;
}

/*
   Retrieve the current log level.
*/
Logger::LogLevel Logger::getLogLevel() {
  return logLevel;
}

/*
   Return a timestamp when the last log entry was made.
*/
uint32_t Logger::getLastLogTime() {
  return lastLogTime;
}

/*
   Returns if debug log level is enabled. This can be used in time critical
   situations to prevent unnecessary string concatenation (if the message won't
   be logged in the end).

   Example:
   if (Logger::isDebug()) {
      Logger::debug("current time: %d", millis());
   }
*/
boolean Logger::isDebug() {
  return logLevel == Debug;
}

void Logger::log(LogLevel level, const char *format, va_list args) {
 
    if (level < Cons){
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

void Logger::logMessage(const char *format, va_list args) {
  char buf[128]; // resulting string limited to 128 chars
  vsnprintf(buf, 128, format, args);
  SERIALCONSOLE.print(buf);
}
