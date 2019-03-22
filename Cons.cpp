#include "Cons.hpp"
#include "CONFIG.h"
#include <stdio.h>
#include <stdarg.h>

#define CONSOLEBUFFERSIZE 64

/////////////////////////////////////////////////
/// \brief This is the consoles main function where commands and interpreted every tick.
/////////////////////////////////////////////////
void Cons::doConsole() {
  static unsigned char y[CONSOLEBUFFERSIZE] = {0};
  static unsigned char yptr = 0;
  static unsigned char numB = 0;
  if (SERIALCONSOLE.available()) {
    numB = SERIALCONSOLE.readBytesUntil('\n', &y[yptr], CONSOLEBUFFERSIZE - 1 - yptr);
    if ((y[yptr] == '\n') || (y[yptr] == '\r')) {
      LOG_CONSOLE("\r\n");
      switch (y[0]) {
        case '1':
          controller_inst_ptr->getBMSPtr()->printPackSummary();
          break;

        case '2':
          controller_inst_ptr->getBMSPtr()->printPackDetails();
          break;

        case '3':
          controller_inst_ptr->getBMSPtr()->printAllCSV();
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
    } else if (yptr < CONSOLEBUFFERSIZE - 3) {
      LOG_CONSOLE("%s", &y[yptr]);
      yptr += numB;
      return;
    }

    //at this point prepare for a new command
    for (int i = 0; i < CONSOLEBUFFERSIZE; i++) {
      y[i] = 0;
    }
    yptr = 0;
    LOG_CONSOLE(">> ");
  }
}

/////////////////////////////////////////////////
/// \brief Prints banner and menu to the console.
/////////////////////////////////////////////////
void Cons::printMenu() {
  LOG_CONSOLE("\n\\||||||||/   \\||||||||/   |||||||||/   ||           \\||||||||/\n");
  LOG_CONSOLE("    ||                    ||           ||\n");
  LOG_CONSOLE("    ||       \\||||||||/   ||||||||||   ||           ||||||||||\n");
  LOG_CONSOLE("    ||                            ||   ||           ||      ||\n");
  LOG_CONSOLE("    ||       \\||||||||/   /|||||||||   |||||||||/   ||      ||\n\n");
  LOG_CONSOLE("              Battery Management System by GuilT\n");
  LOG_CONSOLE("\n************************* SYSTEM MENU *************************\n");
  LOG_CONSOLE("GENERAL SYSTEM CONFIGURATION\n\n");
  LOG_CONSOLE("   h or ? = help (displays this message)\n");
  LOG_CONSOLE("   1 = display BMS status summary\n");
  LOG_CONSOLE("   2 = display BMS detailed summary\n");
  LOG_CONSOLE("   3 = output BMS details in CSV format\n");
  LOG_CONSOLE("   vX verbose (X=0:debug, X=1:info, X=2:warn, X=3:error, X=4:Cons)\n");
  LOG_CONSOLE("\n");
}

/////////////////////////////////////////////////
/// \brief Constructor
/////////////////////////////////////////////////
Cons::Cons(Controller* cont_inst_ptr) {
  // initialize serial communication at 9600 bits per second:
  SERIALCONSOLE.begin(115200);
  SERIALCONSOLE.setTimeout(15);
  controller_inst_ptr = cont_inst_ptr;
}
