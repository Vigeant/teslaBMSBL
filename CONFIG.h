#pragma once

#include <Arduino.h>

//enable testing mode that simply cycles through all states instead of triggers
//#define STATECYCLING 1

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
//                            12

//side 2
#define OLED_PIN_SCK          13    //LED
//                            14
#define OLED_PIN_RESET        15
#define INL_BAT_PACK_FAULT    16    //Tesla Battery pack fault.
#define INL_BAT_MON_FAULT     17    //Battery Monitor Fault.
#define INL_EVSE_DISC         18    //Electric Vehicle Supply Equipment Disconnected (from EVCC).
#define INH_RUN               19    //RUN signal from power relay with voltage divider from 12V to 5V.
#define INH_CHARGING          20    //CHARGING signal from EVCC with voltage divider from 12V to 5V.
#define INA_12V_BAT           A7    //PIN21 12v battery monitor. Analog input with 12V to 5V voltage divider.
#define OUTL_EVCC_ON          22    //drive low to power on EVCC. Cycling this signal will force a new charge cycle.
#define OUTL_NO_FAULT         23    //drive low to signal no fault to EVCC. Required for EVCC to charge.

//top side
//short P and G to reset board into program mode using push button

//Set to the proper port for your USB connection - SerialUSB on Due (Native) or Serial for Due (Programming) or Teensy
#define SERIALCONSOLE   Serial

#define VERSION 1
#define CANBUS_SPEED 500000
#define CANBUS_ADDRESS 1
#define OVER_V_SETPOINT 4.2f
#define UNDER_V_SETPOINT 3.0f
//stop charging (if possible)
#define MAX_CHARGE_V_SETPOINT 4.1f
//cycle charger to force a charging cycle
#define CHARGER_CYCLE_V_SETPOINT 3.9f
//issue a warning on OLED and serial console if a cell is that close to a OV or UV fault.
#define WARN_CELL_V_OFFSET 0.1f

/*
 * cooling system setings
 */
#define FLOOR_DUTY_COOLANT_PUMP 0.25f // 0.0 - 1.0
#define COOLING_LOWT_SETPOINT 25.0f  //threashold at wich coolant pump gradually increases duty up to max.
#define COOLING_HIGHT_SETPOINT 35.0f //threshold at wich coolant pump is at maximum duty

#define OVER_T_SETPOINT 40.0f       //Tesla seam to allow reaching 45C while supercharging
#define UNDER_T_SETPOINT -10.0f
//issue a warning on OLED and serial console if T is that close to a OT or UT fault.
#define WARN_T_OFFSET 5.0f
//start precision balancing when highest cell reaches this setpoint (taken from tom debree)
#define PRECISION_BALANCE_V_SETPOINT 3.9f
//precision balance all cells above the lowest cell by this offset (taken from tom debree)
#define PRECISION_BALANCE_CELL_V_OFFSET 0.04f
//start rough balancing when highest cell reaches this setpoint (taken from tom debree)
#define ROUGH_BALANCE_V_SETPOINT 3.6f
//rough balance all cells above the lowest cell by this offset (taken from tom debree)
#define ROUGH_BALANCE_CELL_V_OFFSET 0.10f
//DC 2 DC 12V battery charging cycle trigger
#define DC2DC_CYCLE_V_SETPOINT 12.0f
//DC 2 DC 12V battery charging cycle time in seconds
#define DC2DC_CYCLE_TIME_S 3600
//12V battery OV setpoint
#define BAT12V_OVER_V_SETPOINT 14.5f
//12V battery UV setpoint
#define BAT12V_UNDER_V_SETPOINT 10.0f
//12V battery ADC deviSor 0-1023 -> 0-15V
#define BAT12V_SCALING_DIVISOR 68.0f
