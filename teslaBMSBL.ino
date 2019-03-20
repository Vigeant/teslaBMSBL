//#include <Arduino.h>
#include "Cons.hpp"
#include "Logger.hpp"
#include "Oled.hpp"
#include "Controller.hpp"


//instantiate the console
static Controller controller_inst;
static Cons cons_inst;
static Oled oled_inst(&controller_inst);

// the setup function runs once when you press reset or power the board
void setup() {

  //console stuff
  pinMode(INL_SOFT_RST, INPUT_PULLUP);
  cons_inst.printMenu();
  LOG_CONSOLE(">> ");
}


void phase20hz(){
    if (digitalRead(INL_SOFT_RST) == LOW) {
      _reboot_Teensyduino_();
    }
    cons_inst.doConsole();
}

void phase10hzA(){
  controller_inst.doController();
}

void phase10hzB(){
  oled_inst.doOled();
}

void loop()
{
  //console stuff
  for (;;) {
    phase20hz();
    phase10hzA();
    delay(50);
    phase20hz();
    phase10hzB();
    delay(50);
  }
}
