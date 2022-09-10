#pragma once

#include <Arduino.h>
//#include <EZPROM.h>

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
#define OUTL_12V_BAT_CHRG     2     //Drive low to turn the DC2DC converter to charge the 12V battery       
#define CAN_TX                3
#define CAN_RX                4
#define OLED_PIN_DC           5
#define INL_SOFT_RST          6     // soft reset of the teensy
#define SERIAL3_RX            7     // from Tesla BMS
#define SERIAL3_TX            8     // to Tesla BMS
#define OUTPWM_PUMP           9     // PWM to coolant pump
#define OLED_PIN_CS           10
#define OLED_PIN_MOSI         11
#define INL_WATER_SENS1       12    // battery enclosure water sensor 1 

//side 2
#define OLED_PIN_SCK          13    //LED
#define INL_WATER_SENS2       14    // battery enclosure water sensor 2 
#define OLED_PIN_RESET        15
#define INL_BAT_PACK_FAULT    16    //Tesla Battery pack fault.
#define INL_BAT_MON_FAULT     17    //Battery Monitor Fault.
#define INL_EVSE_DISC         18    //Electric Vehicle Supply Equipment Disconnected (from EVCC). (set Out2)
#define INH_RUN               19    //RUN signal from power relay with voltage divider from 12V to 3.3V.
#define INH_CHARGING          20    //CHARGING signal from EVCC. (Set Out1 to charging switched 12V)
#define INA_12V_BAT           A7    //PIN21 12v battery monitor. Analog input with 12V to 3.3V voltage divider.
#define OUTL_EVCC_ON          22    //drive low to power on EVCC. Cycling this signal will force a new charge cycle.
#define OUTH_FAULT            23    //drive low to signal no fault to EVCC. Required for EVCC to charge.

//top side
//short P and G to reset board into program mode using push button

//Set to the proper port for your USB connection - SerialUSB on Due (Native) or Serial for Due (Programming) or Teensy
#define SERIALCONSOLE   Serial

/*
   State machine
*/
// Loop periods for the state machine of the controller
#define LOOP_PERIOD_ACTIVE_MS 200
#define LOOP_PERIOD_STANDBY_MS 2500



#define VERSION 1
#define CANBUS_SPEED 250000
#define CANBUS_ADDRESS 1
#define OVER_V_SETPOINT 4.25f
#define UNDER_V_SETPOINT 3.0f
//stop charging
#define MAX_CHARGE_V_SETPOINT 4.2f
//cycle charger to force a charging cycle
#define CHARGER_CYCLE_V_SETPOINT 4.17f
//transition to trickle charging when highest cell reaches this value.
#define TRICKLE_CHARGE_V_SETPOINT 4.19f
//issue a warning on OLED and serial console if a cell is that close to a OV or UV fault.
#define WARN_CELL_V_OFFSET 0.1f

/*
   cooling system setings
*/
#define FLOOR_DUTY_COOLANT_PUMP 0.25f // 0.0 - 1.0
#define COOLING_LOWT_SETPOINT 25.0f  //threashold at wich coolant pump gradually increases duty up to max.
#define COOLING_HIGHT_SETPOINT 35.0f //threshold at wich coolant pump is at maximum duty
#define OVER_T_SETPOINT 45.0f       //Tesla seam to allow reaching 45C while supercharging; in discharge, 60C is ok.
#define UNDER_T_SETPOINT -10.0f
//issue a warning on OLED and serial console if T is that close to a OT or UT fault.
#define WARN_T_OFFSET 5.0f
//start precision balancing when highest cell reaches this setpoint (taken from tom debree)
#define PRECISION_BALANCE_V_SETPOINT 4.0f
//precision balance all cells above the lowest cell by this offset (taken from tom debree)
#define PRECISION_BALANCE_CELL_V_OFFSET 0.005f
//start rough balancing when highest cell reaches this setpoint
#define ROUGH_BALANCE_V_SETPOINT 3.4f
//rough balance all cells above the lowest cell by this offset
#define ROUGH_BALANCE_CELL_V_OFFSET 0.10f
//DC 2 DC 12V battery charging cycle trigger
#define DC2DC_CYCLE_V_SETPOINT 12.5f
//DC 2 DC 12V battery charging cycle time in seconds
#define DC2DC_CYCLE_TIME_S 3600
//12V battery OV setpoint
#define BAT12V_OVER_V_SETPOINT 14.5f
//12V battery UV setpoint
#define BAT12V_UNDER_V_SETPOINT 10.0f
//12V battery ADC devisor 0-1023 -> 0-15V
#define BAT12V_SCALING_DIVISOR 61.78f

class Param {
  public:
    virtual void prettyPrint();
  protected:
    const char* paramName;
    bool editable = true;
    const char* description;
};

template <class C> class ParamImpl : public Param {
  public:
    ParamImpl(const char* pn, bool edit, C v, C vdef, C vmin, C vmax, const char* desc) {
      paramName = pn;
      editable = edit;
      value = v;
      valueDefault = vdef;
      valueMin = vmin;
      valueMax = vmax;
      description = desc;
    }
    void prettyPrint() ;
  private:
    C value;
    C valueDefault;
    C valueMin;
    C valueMax;
};



//class Settings : public EZPROM::Serializable {
class Settings {
  public:
    static const uint8_t number_of_params = 25;
    void printSettings() {
      for (uint32_t i = 0; i < number_of_params; i++) {
        parameters[i]->prettyPrint();
      }
    }
    Settings () {
      //TODO check EEPROM for initialisation and version
      //if no match push defaults to eeprom
      //load config from eeprom

      uint8_t i = 0;
      //                                    name, editable, value, default, min, max
      parameters[i++] = new ParamImpl<uint32_t>("magic_bytes", false, 0xdeadbeef, 0xdeadbeef, 0, 0, "Magic byte to identify eeprom was initialized");
      parameters[i++] = new ParamImpl<uint32_t>("version", false, 1, 1, 0, 0, "eeprom version");
      parameters[i++] = new ParamImpl<uint32_t>("canbus_speed", true, 250000, 250000, 0, 0, "Can bus speed");
      parameters[i++] = new ParamImpl<uint32_t>("canbus_address", true, 1, 1, 0, 0, "Can bus address");
      parameters[i++] = new ParamImpl<float>("over_v_setpoint", true, 4.25f, 4.25f, 3.8f, 4.25f, "Triggers Over V error");
      parameters[i++] = new ParamImpl<float>("under_v_setpoint", true, 3.0f, 3.0f, 2.5f, 3.5f, "Triggers Under V error");
      parameters[i++] = new ParamImpl<float>("max_charge_v_setpoint", true, 4.2f, 4.2f, 3.8f, 4.25f, "Stops charging");
      parameters[i++] = new ParamImpl<float>("charger_cycle_v_setpoint", true, 4.17f, 4.17f, 2.5f, 4.25f, "cycle charger to force a charging cycle");
      parameters[i++] = new ParamImpl<float>("trickle_charge_v_setpoint", true, 4.19f, 4.19f, 2.5f, 4.25f, "transition to trickle charging when highest cell reaches this value");
      parameters[i++] = new ParamImpl<float>("warn_cell_v_offset", true, 0.1f, 0.1f, 0.00f, 3.0f, "TODO, issue a warning on OLED and serial console if a cell is that close to a OV or UV fault");
      parameters[i++] = new ParamImpl<float>("floor_duty_coolant_pump", true, 0.25f, 0.25f, 0.00f, 1.0f, "Lowest pump duty cucle when in RUN or CHARGING");
      parameters[i++] = new ParamImpl<float>("cooling_lowt_setpoint", true, 25.0f, 25.0f, -20.0f, 65.0f, "threashold at wich coolant pump gradually increases duty up to max");
      parameters[i++] = new ParamImpl<float>("cooling_hight_setpoint", true, 35.0f, 35.0f, -20.0f, 65.0f, "threshold at wich coolant pump is at maximum duty");
      parameters[i++] = new ParamImpl<float>("over_t_setpoint", true, 35.0f, 35.0f, 40.0f, 60.0f, "Triggers Over T error; Tesla seam to allow reaching 45C while supercharging; in discharge, 60C is ok");
      parameters[i++] = new ParamImpl<float>("under_t_setpoint", true, -10.0f, -10.0f, -40.0f, 10.0f, "Triggers Under T error");
      parameters[i++] = new ParamImpl<float>("warn_t_offset", true, 5.0f, 5.0f, 0.5f, 10.0f, "TODO, issue a warning on OLED and serial console if T is that close to a OT or UT fault");
      parameters[i++] = new ParamImpl<float>("precision_balance_v_setpoint", true, 4.0f, 4.0f, 3.0f, 4.2f, "start precision balancing when highest cell reaches this setpoint (taken from tom debree)");
      parameters[i++] = new ParamImpl<float>("precision_balance_cell_v_offset", true, 0.005f, 0.005f, 0.001f, 0.1f, "precision balance all cells above the lowest cell by this offset (taken from tom debree)");
      parameters[i++] = new ParamImpl<float>("rough_balance_v_setpoint", true, 3.4f, 3.4f, 3.0f, 4.0f, "start rough balancing when highest cell reaches this setpoint");
      parameters[i++] = new ParamImpl<float>("rough_balance_cell_v_offset", true, 0.1f, 0.1f, 0.05f, 0.5f, "rough balance all cells above the lowest cell by this offset");
      parameters[i++] = new ParamImpl<float>("dc2dc_cycle_v_setpoint", true, 12.5f, 12.5f, 12.0f, 13.0f, "DC 2 DC 12V battery charging cycle trigger");
      parameters[i++] = new ParamImpl<uint32_t>("dc2dc_cycle_time_s", true, 3600, 3600, 60, 14400, "DC 2 DC 12V battery charging cycle time in seconds");
      parameters[i++] = new ParamImpl<float>("bat12v_over_v_setpoint", true, 14.5f, 14.5f, 13.0f, 15.0f, "Triggers 12V battery OV error");
      parameters[i++] = new ParamImpl<float>("bat12v_under_v_setpoint", true, 10.0f, 10.0f, 9.0f, 12.5f, "Triggers 12V battery UV error");
      parameters[i++] = new ParamImpl<float>("bat12v_scaling_divisor", true, 61.78f, 61.78f, 50.0f, 70.0f, "12V battery ADC devisor 0-1023 -> 0-15V");
    }
  private:
    Param * parameters[number_of_params];

};
