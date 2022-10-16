#include <Arduino.h>
#include "Config.hpp"
#include "BMSModuleManager.hpp"
#include <list>
#include <String>

#ifndef CONTROLLER_HPP_
#define CONTROLLER_HPP_

class Fault {
public:
  Fault(const String name, const String code, bool chargeFault, bool runFault, const String msgAsserted, const String msgDeAsserted)
    : name(name),
      code(code),
      fault(false),
      sFault(false),
      chargeFault(chargeFault),
      runFault(runFault),
      msgAsserted(msgAsserted),
      msgDeAsserted(msgDeAsserted),
      debounceCounter(0),
      timeStamp(0) {
    ;
  }

  void countFault(uint32_t fault_debounce_counter) {
    debounceCounter++;
    if (debounceCounter >= fault_debounce_counter) {
      if (fault) {
        LOG_ERR(msgAsserted.c_str());
      }
      fault = true;
      sFault = true;
      timeStamp = now();
      debounceCounter = fault_debounce_counter;
    }
  }
  void resetFault() {
    if (fault) {
      LOG_ERR(msgDeAsserted.c_str());
    }
    fault = false;
    debounceCounter = 0;
  }
  bool getFault() {
    return fault;
  }
  bool getSFault() {
    return sFault;
  }
  const String getName() {
    return name;
  }
  const String getCode() {
    return code;
  }
  bool getChargeFault() {
    return chargeFault;
  }
  bool getRunFault() {
    return runFault;
  }
  const String getMsgAsserted() {
    return msgAsserted;
  }
  const String getMsgDeAsserted() {
    return msgDeAsserted;
  }
  time_t getTimeStamp() {
    return timeStamp;
  }

private:
  const String name;
  const String code;
  bool fault;
  bool sFault;
  bool chargeFault;
  bool runFault;
  const String msgAsserted;
  const String msgDeAsserted;
  uint8_t debounceCounter;
  time_t timeStamp;
};

class Controller {
public:
  enum ControllerState {
    INIT,
    STANDBY,
    PRE_CHARGE,
    CHARGING,
    TRICKLE_CHARGING,
    POST_CHARGE,
    RUN
  };
  void doController();
  Controller();
  ControllerState getState();
  BMSModuleManager* getBMSPtr();
  Settings* getSettingsPtr();
  void printControllerState();
  uint32_t getPeriodMillis();
  int32_t reloadDefaultSettings();
  int32_t saveSettings();

  Fault faultModuleLoop;
  Fault faultBatMon;
  Fault faultBMSSerialComms;
  Fault faultBMSOV;
  Fault faultBMSUV;
  Fault faultBMSOT;
  Fault faultBMSUT;
  Fault fault12VBatOV;
  Fault fault12VBatUV;
  Fault faultWatSen1;
  Fault faultWatSen2;
  Fault faultIncorectModuleCount;

  bool isFaulted;
  bool stickyFaulted;
  float bat12vVoltage;
  bool outL_12V_bat_chrg_buffer;
  uint8_t outpwm_pump_buffer;
  bool outL_evcc_on_buffer;
  bool outH_fault_buffer;

  std::list<Fault*> faults;

private:
  Settings settings;
  BMSModuleManager bms;
  bool chargerInhibit;
  bool powerLimiter;
  bool dc2dcON_H;
  uint32_t period;
  ControllerState state;

  //run-time functions
  void syncModuleDataObjects();  //gathers all the data from the boards and populates the BMSModel object instances
  void balanceCells();           //balances the cells according to thresholds in the BMSModuleManager
  void assertFaultLine();
  void clearFaultLine();
  float getCoolingPumpDuty(float);
  void setOutput(int pin, int state);
  void init();  //reset all boards and assign address to each board
  void standby();
  void pre_charge();
  void charging();
  void trickle_charging();
  void post_charge();
  void run();
};

#endif /* CONTROLLER_H_ */