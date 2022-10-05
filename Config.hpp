#pragma once

#include <Arduino.h>
#include <EEPROM.h>
#include <list>

//enable testing mode that simply cycles through all states instead of triggers
//#define STATECYCLING 1
#define TESTING_MODE 0

#define FAULT_DEBOUNCE_COUNT 3

///////////////////////////////////
// Teensy pin configuration      //
///////////////////////////////////
//side 1
//#define SERIAL1_RX          0
//#define SERIAL1_TX          1
#define OUTL_12V_BAT_CHRG 2  //Drive low to turn the DC2DC converter to charge the 12V battery
#define CAN_TX 3
#define CAN_RX 4
#define OLED_PIN_DC 5
#define INL_SOFT_RST 6  // soft reset of the teensy
#define SERIAL3_RX 7    // from Tesla BMS
#define SERIAL3_TX 8    // to Tesla BMS
#define OUTPWM_PUMP 9   // PWM to coolant pump
#define OLED_PIN_CS 10
#define OLED_PIN_MOSI 11
#define INL_WATER_SENS1 12  // battery enclosure water sensor 1

//side 2
#define OLED_PIN_SCK 13     //LED
#define INL_WATER_SENS2 14  // battery enclosure water sensor 2
#define OLED_PIN_RESET 15
#define INL_BAT_PACK_FAULT 16  //Tesla Battery pack fault.
#define INL_BAT_MON_FAULT 17   //Battery Monitor Fault.
#define INL_EVSE_DISC 18       //Electric Vehicle Supply Equipment Disconnected (from EVCC). (set Out2)
#define INH_RUN 19             //RUN signal from power relay with voltage divider from 12V to 3.3V.
#define INH_CHARGING 20        //CHARGING signal from EVCC. (Set Out1 to charging switched 12V)
#define INA_12V_BAT A7         //PIN21 12v battery monitor. Analog input with 12V to 3.3V voltage divider.
#define OUTL_EVCC_ON 22        //drive low to power on EVCC. Cycling this signal will force a new charge cycle.
#define OUTH_FAULT 23          //drive low to signal no fault to EVCC. Required for EVCC to charge.

//top side
//short P and G to reset board into program mode using push button

//Set to the proper port for your USB connection - SerialUSB on Due (Native) or Serial for Due (Programming) or Teensy
#define SERIALCONSOLE Serial

/*
   State machine
*/
// Loop periods for the state machine of the controller
#define LOOP_PERIOD_ACTIVE_MS 200
#define LOOP_PERIOD_STANDBY_MS 2500

#define EEPROM_VERSION 2

class Param {
public:
  Param(const char* paramName, bool editable, const char* description)
    : paramName(paramName),
      editable(editable),
      description(description) {
    ;
  }
  const char* paramName;
  virtual void prettyPrint();
  virtual int32_t setVal(const char* valStr);
  virtual uint16_t getSize();
  virtual void resetDefault();
  virtual uint32_t saveToEEPROM(uint32_t address);
  virtual uint32_t loadFromEEPROM(uint32_t address);
protected:
  bool editable = true;
  const char* description;
};

template<class C> class ParamImpl : public Param {
public:
  ParamImpl(const char* paramName, bool editable, C value, C valueDefault, C valueMin, C valueMax, const char* description)
    : Param(paramName, editable, description),
      value(value),
      valueDefault(valueDefault),
      valueMin(valueMin) {
    ;
  }

  void prettyPrint();
  int32_t setVal(const char*);

  C getVal() {
    return value;
  }

  void resetDefault() {
    value = valueDefault;
  }

  uint16_t getSize();
  bool valueMatchDefault() {
    return (value == valueDefault);
  }

  uint32_t saveToEEPROM(uint32_t address) {
    EEPROM.put(address, value);
    return sizeof(C);
  }

  uint32_t loadFromEEPROM(uint32_t address) {
    EEPROM.get(address, value);
    return sizeof(C);
  }

private:
  C value;
  C valueDefault;
  C valueMin;
  C valueMax;
};

class Settings {
public:
  void printSettings();
  Settings();
  Param* getParam(const char* name);
  uint16_t size();
  void reloadDefaultSettings();
  void saveAllSettingsToEEPROM(uint32_t address);
  void loadAllSettingsFromEEPROM(uint32_t address);

  ParamImpl<uint32_t> magic_bytes;
  ParamImpl<uint32_t> eeprom_version;
  ParamImpl<uint32_t> canbus_speed;
  ParamImpl<uint32_t> canbus_address;
  ParamImpl<float> over_v_setpoint;
  ParamImpl<float> under_v_setpoint;
  ParamImpl<float> max_charge_v_setpoint;
  ParamImpl<float> charger_cycle_v_setpoint;
  ParamImpl<float> trickle_charge_v_setpoint;
  ParamImpl<float> warn_cell_v_offset;
  ParamImpl<float> floor_duty_coolant_pump;
  ParamImpl<float> cooling_lowt_setpoint;
  ParamImpl<float> cooling_hight_setpoint;
  ParamImpl<float> over_t_setpoint;
  ParamImpl<float> under_t_setpoint;
  ParamImpl<float> warn_t_offset;
  ParamImpl<float> precision_balance_v_setpoint;
  ParamImpl<float> precision_balance_cell_v_offset;
  ParamImpl<float> rough_balance_v_setpoint;
  ParamImpl<float> rough_balance_cell_v_offset;
  ParamImpl<float> dc2dc_cycle_v_setpoint;
  ParamImpl<uint32_t> dc2dc_cycle_time_s;
  ParamImpl<float> bat12v_over_v_setpoint;
  ParamImpl<float> bat12v_under_v_setpoint;
  ParamImpl<float> bat12v_scaling_divisor;
  ParamImpl<uint32_t> fault_debounce_count;
  ParamImpl<uint32_t> module_count;

private:
  std::list<Param*> parameters;
};