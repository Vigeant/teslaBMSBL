#include <FlexCAN.h>
//#include <kinetis_flexcan.h>

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


/*
* The EVCC supports 250kbps CAN data rate and 29 bit identifiers
*/
/*
* BMS->EVCC message Identifier
*/
#define BMS_EVCC_STATUS_IND 0x01dd0001
/* bBMSStatusFlag bits */
#define BMS_STATUS_CELL_HVC_FLAG 0x01 /* set if a cell is > HVC */
#define BMS_STATUS_CELL_LVC_FLAG 0x02 /* set if a cell is < LVC */
#define BMS_STATUS_CELL_BVC_FLAG 0x04 /* set if a cell is > BVC */
/* bBMSFault bits */
#define BMS_FAULT_OVERTEMP_FLAG 0x04 /* set for thermistor overtemp */
/*
* BMS->EVCC message body
*/
typedef struct tBMS_EVCC_StatusInd {
  uint8_t bBMSStatusFlags; /* see bit definitions above */
  uint8_t bBmsId;          /* reserved, set to 0 */
  uint8_t bBMSFault;
  uint8_t bReserved2; /* reserved, set to 0 */
  uint8_t bReserved3; /* reserved, set to 0 */
} tBMS_EVCC_StatusInd;

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
  bool canOn = false;

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

  CAN_message_t msg;
  tBMS_EVCC_StatusInd msgStatusIns;
  
};

#endif /* CONTROLLER_H_ */