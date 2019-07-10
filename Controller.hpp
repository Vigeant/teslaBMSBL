#include <Arduino.h>
#include "CONFIG.h"
#include "BMSModuleManager.hpp"

#ifndef CONTROLLER_HPP_
#define CONTROLLER_HPP_

class Controller {
  public:
    enum ControllerState {
      INIT, STANDBY, PRE_CHARGE, CHARGING, RUN
    };
    void doController();
    Controller();
    ControllerState getState();
    BMSModuleManager* getBMSPtr();
    void printControllerState();
    uint32_t getPeriodMillis();

    //faults
    bool faultModuleLoop;
    bool faultBatMon;
    bool faultBMSSerialComms;
    bool faultBMSOV;
    bool faultBMSUV;
    bool faultBMSOT;
    bool faultBMSUT;
    bool fault12VBatOV;
    bool fault12VBatUV;
    bool faultWatSen1;
    bool faultWatSen2;

    //sticky faults
    bool sFaultModuleLoop;
    bool sFaultBatMon;
    bool sFaultBMSSerialComms;
    bool sFaultBMSOV;
    bool sFaultBMSUV;
    bool sFaultBMSOT;
    bool sFaultBMSUT;
    bool sFault12VBatOV;
    bool sFault12VBatUV;
    bool sFaultWatSen1;
    bool sFaultWatSen2;

    //faults debounce counters (DB)
    uint8_t faultModuleLoopDB;
    uint8_t faultBatMonDB;
    uint8_t faultBMSSerialCommsDB;
    uint8_t faultBMSOVDB;
    uint8_t faultBMSUVDB;
    uint8_t faultBMSOTDB;
    uint8_t faultBMSUTDB;
    uint8_t fault12VBatOVDB;
    uint8_t fault12VBatUVDB;
    uint8_t faultWatSen1DB;
    uint8_t faultWatSen2DB;

    //faults time stamps (TS)
    uint32_t faultModuleLoopTS;
    uint32_t faultBatMonTS;
    uint32_t faultBMSSerialCommsTS;
    uint32_t faultBMSOVTS;
    uint32_t faultBMSUVTS;
    uint32_t faultBMSOTTS;
    uint32_t faultBMSUTTS;
    uint32_t fault12VBatOVTS;
    uint32_t fault12VBatUVTS;
    uint32_t faultWatSen1TS;
    uint32_t faultWatSen2TS;

    bool isFaulted;
    bool stickyFaulted;
    float bat12vVoltage;

  private:

    BMSModuleManager bms;

    bool chargerInhibit;
    bool powerLimiter;
    bool dc2dcON;
    uint32_t period;

    //run-time functions
    void syncModuleDataObjects(); //gathers all the data from the boards and populates the BMSModel object instances
    void balanceCells(); //balances the cells according to thresholds in the BMSModuleManager

    void assertFaultLine();
    void clearFaultLine();
    float getCoolingPumpDuty(float);

    ControllerState state;
    void init(); //reset all boards and assign address to each board
    void standby();
    void pre_charge();
    void charging();
    void run();

};

#endif /* CONTROLLER_H_ */
