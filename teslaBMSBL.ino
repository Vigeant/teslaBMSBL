/**@file teslaBMSBL.ino */
#include <Arduino.h>
#include "Cons.hpp"
#include "Logger.hpp"
#include "Oled.hpp"
#include "Controller.hpp"

#define CPU_RESTART_ADDR (uint32_t *)0xE000ED0C
#define CPU_RESTART_VAL 0x5FA0004
#define CPU_RESTART (*CPU_RESTART_ADDR = CPU_RESTART_VAL);

/*! \mainpage teslaBMSBL
 *
 * \section intro_sec Introduction
 *
 * A teensy based battery management system for Tesla battery modules.
 *
 * \section install_sec Installation
 *
 * \subsection step1 Step 1: Opening the box
 *  
 * etc...
 */

//instantiate all objects
static Controller controller_inst;        ///< The controller is responsible for orchestrating all major functions of the BMS.
static Cons cons_inst(&controller_inst);  ///< The console is a 2 way user interface available on usb serial port at baud 115200.
static Oled oled_inst(&controller_inst);  ///< The oled is a 1 way user interface displaying the most critical information.

/////////////////////////////////////////////////
/// \brief The setup function runs once when you press reset or power the board.
/////////////////////////////////////////////////
void setup() {
  //console stuff
  pinMode(INL_SOFT_RST, INPUT_PULLUP);
  cons_inst.printMenu();
  LOG_CONSOLE(">> ");
}

/////////////////////////////////////////////////
/// Holds all the code that runs every 50ms.
/////////////////////////////////////////////////
void phase20hz() {
  if (digitalRead(INL_SOFT_RST) == LOW) {
    //_reboot_Teensyduino_();
    CPU_RESTART;
  }
  cons_inst.doConsole();
}

/////////////////////////////////////////////////
/// Holds code that runs every 100ms.
/////////////////////////////////////////////////
void phase10hzA() {
  controller_inst.doController();
}

/////////////////////////////////////////////////
/// Holds code that runs every 100ms.
/////////////////////////////////////////////////
void phase10hzB() {
  oled_inst.doOled();
}

/////////////////////////////////////////////////
/// Once setup is complete, loop is called for ever.
/////////////////////////////////////////////////
void loop()
{
  uint32_t starttime, endtime, delaytime, timespent;
  uint32_t period = 200;
  bool phaseA = true;
  
  for (;;) {
    starttime = millis();

    phase20hz();
    if (phaseA)
      phase10hzA();
    else
      phase10hzB();
    phaseA = !phaseA;    

    endtime = millis();
    if (endtime > starttime) {
      timespent = endtime - starttime;
    } else {
      starttime = 0xffffffff - starttime;
      timespent = starttime + endtime;
    }
    if (timespent >= period) {
      delaytime = 0;
    } else {
      delaytime = period - timespent;
    }
    delay(delaytime);
  }
}
