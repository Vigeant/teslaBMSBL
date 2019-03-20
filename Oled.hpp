#include <Arduino.h>
#include "CONFIG.h"
#include "Controller.hpp"

#include <TeensyView.h>  // Include the SFE_TeensyView library

class Oled {
public:

  Oled(Controller*);
  void doOled();
  //static void init();

private:
  enum formatState {
      FMT1 = 0, FMT2 = 1, FMT3 = 2, FMT4 = 3, FMT5 = 4, FMT6 = 5, FMT7 = 6, FMT8 = 7, FMT9 = 8
  };
  formatState state;
  Controller* controller_inst_ptr;
  void printFormat1();
  void printFormat2();
  void printFormat3();
  void printFormat4();
  void printFormat5();
  void printTeslaBMSRT();
  void printESidewinder();
  void printFaults();
  void printStickyFaults();
  
  void printCentre(const char*  , int);
};
