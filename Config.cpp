#include "Config.hpp"
#include <string>
#include <errno.h>

Settings::Settings()
  : magic_bytes("magic_bytes", false, 0, 0xdeadbeef, 0, 0, "Magic byte to identify eeprom was initialized"),
    eeprom_version("eeprom_version", false, 0, EEPROM_VERSION, 0, 0, "eeprom version"),
    canbus_speed("canbus_speed", true, 0, 250000, 62500, 1000000, "Can bus speed"),
    canbus_address("canbus_address", true, 0, 1, 1, 1000, "Can bus address"),
    over_v_setpoint("over_v_setpoint", true, 0.0f, 4.25f, 3.8f, 4.25f, "Triggers Over V error"),
    under_v_setpoint("under_v_setpoint", true, 0.0f, 3.0f, 2.5f, 3.5f, "Triggers Under V error"),
    max_charge_v_setpoint("max_charge_v_setpoint", true, 0.0f, 4.2f, 3.8f, 4.25f, "Stops charging"),
    charger_cycle_v_setpoint("charger_cycle_v_setpoint", true, 0.0f, 4.17f, 2.5f, 4.25f, "Voltage treshold to force a charging cycle"),
    trickle_charge_v_setpoint("trickle_charge_v_setpoint", true, 0.0f, 4.19f, 2.5f, 4.25f, "Transition to trickle charging when highest cell reaches this value"),
    warn_cell_v_offset("warn_cell_v_offset", true, 0.0f, 0.1f, 0.00f, 3.0f, "TODO, issue a warning on OLED and serial console if a cell is that close to a OV or UV fault"),
    floor_duty_coolant_pump("floor_duty_coolant_pump", true, 0.0f, 0.25f, 0.00f, 1.0f, "Lowest pump duty cucle when in RUN or CHARGING"),
    cooling_lowt_setpoint("cooling_lowt_setpoint", true, 0.0f, 25.0f, -20.0f, 65.0f, "Threshold at which coolant pump gradually increases duty up to max"),
    cooling_hight_setpoint("cooling_hight_setpoint", true, 0.0f, 35.0f, -20.0f, 65.0f, "Threshold at which coolant pump is at maximum duty"),
    over_t_setpoint("over_t_setpoint", true, 0.0f, 35.0f, 30.0f, 60.0f, "Triggers Over temperature error; Tesla allows reaching 45C while supercharging; in discharge, 60C is ok"),
    under_t_setpoint("under_t_setpoint", true, 0.0f, -10.0f, -40.0f, 10.0f, "Triggers Under temperature error"),
    warn_t_offset("warn_t_offset", true, 0.0f, 5.0f, 0.5f, 10.0f, "TODO, issue a warning on OLED and serial console if temp. is that close to a OT or UT fault"),
    precision_balance_v_setpoint("precision_balance_v_setpoint", true, 0.0f, 4.0f, 3.0f, 4.2f, "Start precision balancing when highest cell reaches this setpoint (taken from tom debree)"),
    precision_balance_cell_v_offset("precision_balance_cell_v_offset", true, 0.0f, 0.005f, 0.001f, 0.1f, "Precision balance all cells above the lowest cell by this offset (taken from tom debree)"),
    rough_balance_v_setpoint("rough_balance_v_setpoint", true, 0.0f, 3.4f, 3.0f, 4.0f, "Start rough balancing when highest cell reaches this setpoint"),
    rough_balance_cell_v_offset("rough_balance_cell_v_offset", true, 0.0f, 0.1f, 0.05f, 0.5f, "Rough balance all cells above the lowest cell by this offset"),
    dc2dc_cycle_v_setpoint("dc2dc_cycle_v_setpoint", true, 0.0f, 12.5f, 12.0f, 13.0f, "DC 2 DC 12V battery charging cycle trigger"),
    dc2dc_cycle_time_s("dc2dc_cycle_time_s", true, 0, 3600, 60, 14400, "DC 2 DC 12V battery charging cycle time in seconds"),
    bat12v_over_v_setpoint("bat12v_over_v_setpoint", true, 0.0f, 14.5f, 13.0f, 15.0f, "Triggers 12V battery OV error"),
    bat12v_under_v_setpoint("bat12v_under_v_setpoint", true, 0.0f, 10.0f, 9.0f, 12.5f, "Triggers 12V battery UV error"),
    bat12v_scaling_divisor("bat12v_scaling_divisor", true, 0.0f, 61.78f, 50.0f, 70.0f, "12V battery ADC devisor 0-1023 -> 0-15V"),
    fault_debounce_count("fault_debounce_count", true, 0, 5, 1, 100, "Number of time a fault condition has to be counted before the fault is recorded/asserted"),
    module_count("module_count", true, 0, 7, 1, 64, "triggers an error if we see less than this number of modules."),
    oled_cycle_time("oled_cycle_time", true, 0, 4000, 1000, 50000, "miliseconds per oled screen cycle.") {
  //TODO check EEPROM for initialisation and version
  //if no match push defaults to eeprom
  //load config from eeprom
  parameters.push_back(&magic_bytes);
  parameters.push_back(&eeprom_version);
  parameters.push_back(&canbus_speed);
  parameters.push_back(&canbus_address);
  parameters.push_back(&over_v_setpoint);
  parameters.push_back(&under_v_setpoint);
  parameters.push_back(&max_charge_v_setpoint);
  parameters.push_back(&charger_cycle_v_setpoint);
  parameters.push_back(&trickle_charge_v_setpoint);
  parameters.push_back(&warn_cell_v_offset);
  parameters.push_back(&floor_duty_coolant_pump);
  parameters.push_back(&cooling_lowt_setpoint);
  parameters.push_back(&cooling_hight_setpoint);
  parameters.push_back(&over_t_setpoint);
  parameters.push_back(&under_t_setpoint);
  parameters.push_back(&warn_t_offset);
  parameters.push_back(&precision_balance_v_setpoint);
  parameters.push_back(&precision_balance_cell_v_offset);
  parameters.push_back(&rough_balance_v_setpoint);
  parameters.push_back(&rough_balance_cell_v_offset);
  parameters.push_back(&dc2dc_cycle_v_setpoint);
  parameters.push_back(&dc2dc_cycle_time_s);
  parameters.push_back(&bat12v_over_v_setpoint);
  parameters.push_back(&bat12v_under_v_setpoint);
  parameters.push_back(&bat12v_scaling_divisor);
  parameters.push_back(&fault_debounce_count);
  parameters.push_back(&module_count);
  parameters.push_back(&oled_cycle_time);
}

void Settings::printSettings() {
  Serial.printf("%35s | %-10s | %-10s | [%10s , %-10s] | %s\n", "param name", "value", "default", "min", "max", "description");
  for (auto i = parameters.begin(); i != parameters.end(); i++) {
    (*i)->prettyPrint();
  }
}

void Settings::reloadDefaultSettings() {
  for (auto i = parameters.begin(); i != parameters.end(); i++) {
    (*i)->resetDefault();
  }
}

template<>
void ParamImpl<uint32_t>::prettyPrint() {
  Serial.printf("%35s | %-10u | %-10u | [%10u , %-10u] | %s\n", paramName, value, valueDefault, valueMin, valueMax, description);
}

template<>
void ParamImpl<int32_t>::prettyPrint() {
  Serial.printf("%35s | %-10d | %-10d | [%10d , %-10d] | %s\n", paramName, value, valueDefault, valueMin, valueMax, description);
}

template<>
void ParamImpl<float>::prettyPrint() {
  Serial.printf("%35s | %-10.3f | %-10.3f | [%10.3f , %-10.3f] | %s\n", paramName, value, valueDefault, valueMin, valueMax, description);
}

template<>
int32_t ParamImpl<uint32_t>::setVal(const char* valStr) {
  char* endptr;
  uint32_t tempValue;
  if (!editable) {
    return -1;
  }
  errno = 0;  // To distinguish success/failure after call
  tempValue = strtoul(valStr, &endptr, 10);
  // Check for various possible errors
  if (errno == ERANGE || (errno != 0 && tempValue == 0) || tempValue > valueMax || tempValue < valueMin) {
    return -1;
  }
  // check if digits were found
  if (endptr == valStr) {
    return -2;
  }
  value = tempValue;
  return 0;
}

template<>
int32_t ParamImpl<int32_t>::setVal(const char* valStr) {
  char* endptr;
  int32_t tempValue;
  errno = 0;  // To distinguish success/failure after call
  tempValue = strtol(valStr, &endptr, 10);
  // Check for various possible errors
  if (errno == ERANGE || (errno != 0 && tempValue == 0) || tempValue > valueMax || tempValue < valueMin) {
    return -1;
  }
  // check if digits were found
  if (endptr == valStr) {
    return -2;
  }
  value = tempValue;
  return 0;
}

template<>
uint16_t ParamImpl<int32_t>::getSize() {
  return sizeof(int32_t);
}

template<>
uint16_t ParamImpl<uint32_t>::getSize() {
  return sizeof(uint32_t);
}

template<>
uint16_t ParamImpl<float>::getSize() {
  return sizeof(float);
}

template<>
int32_t ParamImpl<float>::setVal(const char* valStr) {
  char* endptr;
  float tempValue;
  errno = 0;  // To distinguish success/failure after call
  tempValue = strtof(valStr, &endptr);
  // Check for various possible errors
  if (errno == ERANGE || (errno != 0 && tempValue == 0) || tempValue > valueMax || tempValue < valueMin) {
    return -1;
  }
  // check if digits were found
  if (endptr == valStr) {
    return -2;
  }
  value = tempValue;
  return 0;
}

Param* Settings::getParam(const char* name) {
  for (auto i = parameters.begin(); i != parameters.end(); i++) {
    if (strncmp((*i)->paramName, name, strlen((*i)->paramName)) == 0) {
      return *i;
    }
  }
  return 0;
}

void Settings::saveAllSettingsToEEPROM(uint32_t address) {
  for (auto i = parameters.begin(); i != parameters.end(); i++) {
    address += (*i)->saveToEEPROM(address);
  }
}

void Settings::loadAllSettingsFromEEPROM(uint32_t address) {
  for (auto i = parameters.begin(); i != parameters.end(); i++) {
    address += (*i)->loadFromEEPROM(address);
  }
}