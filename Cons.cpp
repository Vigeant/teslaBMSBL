#include "Cons.hpp"
#include "CONFIG.h"
#include <stdio.h>
#include <stdarg.h>

#define CONSOLEBUFFERSIZE 64

/////////////////////////////////////////////////
/// \brief This is the consoles main function where commands and interpreted every tick.
/////////////////////////////////////////////////
void Cons::doConsole() {
  static char y[CONSOLEBUFFERSIZE] = {0};
  static unsigned char lastyptr = 0;
  static unsigned char yptr = 0;
  static unsigned char numB = 0;
  if (SERIALCONSOLE.available()) {
    numB = SERIALCONSOLE.readBytesUntil('\n', &y[lastyptr], CONSOLEBUFFERSIZE - 1 - lastyptr);

    //LOG_CONSOLE("lastyptr: 0x%x, yptr: 0x%x, numB: 0x%x\r\n", lastyptr, yptr, numB);


    while (yptr < lastyptr + numB) {
      if ((y[yptr] == '\n') || (y[yptr] == '\r')) {
        LOG_CONSOLE("\r\n");
        //LOG_CONSOLE("LL%c", y[0]);
        switch (y[0]) {
          case '1':
            LOG_CONSOLE("12V Battery: %.2fV \n", controller_inst_ptr->bat12vVoltage);
            controller_inst_ptr->getBMSPtr()->printPackSummary();
            controller_inst_ptr->printControllerState();
            break;

          case '2':
            controller_inst_ptr->getBMSPtr()->printPackGraph();
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
        memset(y,0,CONSOLEBUFFERSIZE);
        yptr = 0;
        lastyptr = 0;
        LOG_CONSOLE("BMS> ");
        break; //not perfect but...
      } else {
        //LOG_CONSOLE("y[%d]:%c y[0]:0x%x", yptr, y[yptr], y[0]);
        LOG_CONSOLE("%c", y[yptr]);
        yptr++;
      }
    }

    //wait for next tick
    lastyptr = yptr;
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
  LOG_CONSOLE("   2 = print cell voltage graph\n");
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
