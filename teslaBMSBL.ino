/**@file teslaBMSBL.ino */
#include <Arduino.h>
#include "Controller.hpp"
#include "Cons.hpp"
#include "Logger.hpp"
#include "Oled.hpp"
#include <Snooze.h>
#include <TimeLib.h>

#define CPU_RESTART_ADDR (uint32_t *)0xE000ED0C
#define CPU_RESTART_VAL 0x5FA0004
#define CPU_RESTART (*CPU_RESTART_ADDR = CPU_RESTART_VAL);

/*! \mainpage teslaBMSBL

   \section intro_sec Introduction

   A teensy based battery management system for Tesla battery modules.

   \section install_sec Installation

   \subsection step1 Step 1: Opening the box

   etc...
*/

//instantiate all objects
//static Settings settings;
static Controller controller_inst;        ///< The controller is responsible for orchestrating all major functions of the BMS.
static Cons cons_inst(&controller_inst);  ///< The console is a 2 way user interface available on usb serial port at baud 115200.
static Oled oled_inst(&controller_inst);  ///< The oled is a 1 way user interface displaying the most critical information.

// Load drivers
//SnoozeTouch touch;
SnoozeDigital digital;
SnoozeTimer timer;
SnoozeUSBSerial usbSerial;

// install drivers to a SnoozeBlock
SnoozeBlock config(timer, digital, usbSerial);

time_t getTeensy3Time() {
  return Teensy3Clock.get();
}
/////////////////////////////////////////////////
/// \brief The setup function runs once when you press reset or power the board.
/////////////////////////////////////////////////
void setup() {

  pinMode(INL_SOFT_RST, INPUT_PULLUP);

  // set the Time library to use Teensy 3.0's RTC to keep time
  setSyncProvider(getTeensy3Time);
  delay(100);
  if (timeStatus() != timeSet) {
    Serial.println("Unable to sync with the RTC");
  } else {
    Serial.println("RTC has set the system time");
  }

  LOG_CONSOLE("BMS> ");
}


/////////////////////////////////////////////////
/// Holds all the code that runs every period.
/////////////////////////////////////////////////
void phase1main() {
  if (digitalRead(INL_SOFT_RST) == LOW) {
    //_reboot_Teensyduino_();
    CPU_RESTART;
  }
  cons_inst.doConsole();
}

/////////////////////////////////////////////////
/// Holds code that runs every two periods.
/////////////////////////////////////////////////
void phase1A() {
  controller_inst.doController();
}

/////////////////////////////////////////////////
/// Holds code that runs every two periods.
/////////////////////////////////////////////////
void phase1B() {
  oled_inst.doOled();
}

/////////////////////////////////////////////////
/// Once setup is complete, loop is called for ever.
/////////////////////////////////////////////////
void loop() {
  uint32_t starttime, endtime, delaytime, timespent, period;
  bool phaseA = true;

  for (;;) {
    starttime = millis();

    phase1main();
    if (phaseA)
      phase1A();
    else
      phase1B();
    phaseA = !phaseA;

    digital.pinMode(INL_SOFT_RST, INPUT_PULLUP, FALLING);  //pin, mode, type

    //get loop period from controller
    period = controller_inst.getPeriodMillis();

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

    //sleep board instead of delay, if not in active state
    if (delaytime > LOOP_PERIOD_ACTIVE_MS) {
      timer.setTimer(delaytime);  // milliseconds
      //who = Snooze.deepSleep( config );
      (void)Snooze.deepSleep(config);
    } else {
      delay(delaytime);
    }
  }
}