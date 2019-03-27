#include <Arduino.h>
#include "CONFIG.h"
#include "BMSModuleManager.hpp"

#ifndef CONTROLLER_HPP_
#define CONTROLLER_HPP_

class Controller {
  public:
    enum ControllerState {
      INIT, STANDBY, STANDBY_DC2DC, CHARGER_CYCLE, PRE_CHARGE, CHARGING, RUN, EVSE_CONNECTED, EVSE_CONNECTED_DC2DC
    };
    void doController();
    Controller();
    ControllerState getState();
    BMSModuleManager* getBMSPtr();

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

    bool isFaulted;
    bool stickyFaulted;
    float bat12vVoltage;

  private:

    BMSModuleManager bms;

    bool chargerInhibit;
    bool powerLimiter;

    //run-time functions
    void syncModuleDataObjects(); //gathers all the data from the boards and populates the BMSModel object instances
    void balanceCells(); //balances the cells according to thresholds in the BMSModuleManager

    void assertFaultLine();
    void clearFaultLine();
    float getCoolingPumpDuty(float);

    ControllerState state;
    void init(); //reset all boards and assign address to each board
    void standby();
    void standbyDC2DC();
    void evseConnected();
    void evseConnectedDC2DC();
    void cargerCycle();
    void preCharge();
    void charging();
    void run();

};

#endif /* CONTROLLER_H_ */
