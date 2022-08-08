#include <Arduino.h>
#include "CONFIG.h"
#include "BMSModuleManager.hpp"

#ifndef CONTROLLER_HPP_
#define CONTROLLER_HPP_

class Controller {
  public:
    enum ControllerState {
      INIT, STANDBY, PRE_CHARGE, CHARGING, TRICKLE_CHARGING, POST_CHARGE, RUN
    };
    void doController();
    Controller();
    ControllerState getState();
    BMSModuleManager* getBMSPtr();
    void printControllerState();
    uint32_t getPeriodMillis();

    //faults overhaul
    enum faultNames { FModuleLoop, FBatMon, FBMSSerialComms, FBMSOV, FBMSUV, FBMSOT, FBMSUT, F12VBatOV, F12VBatUV, FWatSen1, FWatSen2};

    typedef struct Fault {
      char name[50];
      char code[2];
      bool fault;
      bool sFault;
      bool chargeFault;
      bool runFault;
      char msgAsserted[100];
      char msgDeAsserted[100];
      uint8_t debounceCounter;
      uint32_t timeStamp;
    } fault_t;

    fault_t faults[11] = {
      [0] = { .name = "ModuleLoop", .code = "A", .fault = false, .sFault = false, .chargeFault = true, .runFault = true, .msgAsserted = "One or more BMS modules have asserted the fault loop!\n", .msgDeAsserted = "All modules have deasserted the fault loop\n" },
      [1] = { .name = "BatMon", .code = "B", .fault = false, .sFault = false, .chargeFault = true, .runFault = true, .msgAsserted = "The battery monitor asserted a fault!\n", .msgDeAsserted = "The battery monitor deasserted a fault\n"},
      [2] = { .name = "BMSSerialComms", .code = "C", .fault = false, .sFault = false, .chargeFault = true, .runFault = true, .msgAsserted = "Serial communication with battery modules lost!\n", .msgDeAsserted = "Serial communication with battery modules re-established!\n" },
      [3] = { .name = "BMSOV", .code = "D", .fault = false, .sFault = false, .chargeFault = true, .runFault = true, .msgAsserted = "A cell reached a voltage higher than the OV threshold\n", .msgDeAsserted = "All cells are back under OV threshold\n"},
      [4] = { .name = "BMSUV", .code = "E", .fault = false, .sFault = false, .chargeFault = false, .runFault = true, .msgAsserted = "A cell reached a voltage lower than the UV threshold\n", .msgDeAsserted = "All cells are back over UV threshold\n"},
      [5] = { .name = "BMSOT", .code = "F", .fault = false, .sFault = false, .chargeFault = true, .runFault = true, .msgAsserted = "A module reached a temp higher than the OT threshold\n", .msgDeAsserted = "All modules are back under OT threshold\n"},
      [6] = { .name = "BMSUT", .code = "G", .fault = false, .sFault = false, .chargeFault = true, .runFault = true, .msgAsserted = "A module reached a temp lower than the UT threshold\n", .msgDeAsserted = "All modules are back over UT threshold\n"},
      [7] = { .name = "12VBatOV", .code = "H", .fault = false, .sFault = false, .chargeFault = false, .runFault = false, .msgAsserted = "12V battery reached a voltage higher than the OV threshold\n", .msgDeAsserted = "12V battery back under the OV threshold\n"},
      [8] = { .name = "12VBatUV", .code = "I", .fault = false, .sFault = false, .chargeFault = false, .runFault = true, .msgAsserted = "12V battery reached a voltage lower than the UV threshold\n", .msgDeAsserted = "12V battery back over the UV threshold\n"},
      [9] = { .name = "WatSen1", .code = "J", .fault = false, .sFault = false, .chargeFault = true, .runFault = true, .msgAsserted = "The battery water sensor 1 is reporting water!\n", .msgDeAsserted = "The battery water sensor 1 is reporting dry.\n"},
      [10] = { .name = "WatSen2", .code = "K", .fault = false, .sFault = false, .chargeFault = true, .runFault = true, .msgAsserted = "The battery water sensor 2 is reporting water!\n", .msgDeAsserted = "The battery water sensor 2 is reporting dry.\n"}
    };

    bool isFaulted;
    bool stickyFaulted;
    float bat12vVoltage;

    bool outL_12V_bat_chrg_buffer;
    uint8_t outpwm_pump_buffer;
    bool outL_evcc_on_buffer;
    bool outH_fault_buffer;

  private:

    BMSModuleManager bms;

    bool chargerInhibit;
    bool powerLimiter;
    bool dc2dcON_H;
    uint32_t period;

    //run-time functions
    void assertFault(faultNames fautlName);
    void deAssertFault(faultNames fautlName);
    void syncModuleDataObjects(); //gathers all the data from the boards and populates the BMSModel object instances
    void balanceCells(); //balances the cells according to thresholds in the BMSModuleManager

    void assertFaultLine();
    void clearFaultLine();
    float getCoolingPumpDuty(float);

    ControllerState state;
    void setOutput(int pin, int state);
    void init(); //reset all boards and assign address to each board
    void standby();
    void pre_charge();
    void charging();
    void trickle_charging();
    void post_charge();
    void run();

};

#endif /* CONTROLLER_H_ */
