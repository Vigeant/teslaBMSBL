#include "Cons.hpp"
#include "CONFIG.h"
#include <stdio.h>
#include <stdarg.h>

void Cons::doConsole() {
  static unsigned char y[32] = {0};
  static unsigned char yptr = 0;
  static unsigned char numB = 0;
  if (SERIALCONSOLE.available()) {
    numB = SERIALCONSOLE.readBytesUntil('\n', &y[yptr], 31 - yptr);
    if ((y[yptr] == '\n') || (y[yptr] == '\r')) {
      LOG_CONSOLE("\r\n");
      switch (y[0]) {
        case '1':
          LOG_CONSOLE("Enable debug mode to see stack usage for all tasks\n");
          break;

        case '2':
          LOG_CONSOLE("Option %s\n", y);
          LOG_INFO("log_inst@: , loglevel: %d\n", &log_inst, log_inst.getLogLevel());
          break;

        case 'v':
          if (y[1] >= 0x30 && y[1] <= 0x35) {
            log_inst.setLoglevel((Logger::LogLevel)(y[1] - 0x30));
            LOG_DEBUG("logLevel set to :%d\n", log_inst.getLogLevel());
            LOG_INFO("logLevel set to :%d\n", log_inst.getLogLevel());
            LOG_WARN("logLevel set to :%d\n", log_inst.getLogLevel());
            LOG_ERROR("logLevel set to :%d\n", log_inst.getLogLevel());
            LOG_CONSOLE("logLevel set to :%d\n", log_inst.getLogLevel());
          } else {
            LOG_CONSOLE("logLevel out of bounds (0-5)\n");
          }
          break;

        case 'h':
        case '?':
          printMenu();
          break;

        case '\n':
        case '\r':
        default:
          break;
      }
    } else if (yptr < 29) {
      LOG_CONSOLE("%s", &y[yptr]);
      yptr += numB;
      return;
    }

    //at this point prepare for a new command
    for (int i = 0; i < 32; i++) {
      y[i] = 0;
    }
    yptr = 0;
    LOG_CONSOLE(">> ");
  }
}

void Cons::printMenu() {

  LOG_CONSOLE("\n\\||||||||/   \\||||||||/   |||||||||/   ||           \\||||||||/\n");
  LOG_CONSOLE("    ||                    ||           ||\n");
  LOG_CONSOLE("    ||       \\||||||||/   ||||||||||   ||           ||||||||||\n");
  LOG_CONSOLE("    ||                            ||   ||           ||      ||\n");
  LOG_CONSOLE("    ||       \\||||||||/   /|||||||||   |||||||||/   ||      ||\n\n");
  LOG_CONSOLE("          Real Time Battery Management System by GuilT\n");
  LOG_CONSOLE("\n************************* SYSTEM MENU *************************\n");
  LOG_CONSOLE("GENERAL SYSTEM CONFIGURATION\n\n");
  LOG_CONSOLE("   h or ? = help (displays this message)\n");
  LOG_CONSOLE("   1 = display stack high watermark for this task\n");
  LOG_CONSOLE("   vX verbose (X=0:debug, X=1:info, X=2:warn, X=3:error, X=4:Cons)\n");
  LOG_CONSOLE("\n");
}

Cons::Cons() {
  // initialize serial communication at 9600 bits per second:
  SERIALCONSOLE.begin(115200);

  /*
    while (!SERIALCONSOLE) {
    ; // wait for serial port to connect. Needed for native USB, on LEONARDO, MICRO, YUN, and other 32u4 based boards.
    }*/

  SERIALCONSOLE.setTimeout(15);
}
