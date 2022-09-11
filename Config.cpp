#include "Config.hpp"

Settings::Settings () :
  magic_bytes("magic_bytes", false, 0xdeadbeef, 0xdeadbeef, 0, 0, "Magic byte to identify eeprom was initialized"),
  eeprom_version("eeprom_version", false, 1, 1, 0, 0, "eeprom version"),
  canbus_speed("canbus_speed", true, 250000, 250000, 0, 0, "Can bus speed"),
  canbus_address("canbus_address", true, 1, 1, 0, 0, "Can bus address"),
  over_v_setpoint("over_v_setpoint", true, 4.25f, 4.25f, 3.8f, 4.25f, "Triggers Over V error"),
  under_v_setpoint("under_v_setpoint", true, 3.0f, 3.0f, 2.5f, 3.5f, "Triggers Under V error"),
  max_charge_v_setpoint("max_charge_v_setpoint", true, 4.2f, 4.2f, 3.8f, 4.25f, "Stops charging"),
  charger_cycle_v_setpoint("charger_cycle_v_setpoint", true, 4.17f, 4.17f, 2.5f, 4.25f, "cycle charger to force a charging cycle"),
  trickle_charge_v_setpoint("trickle_charge_v_setpoint", true, 4.19f, 4.19f, 2.5f, 4.25f, "transition to trickle charging when highest cell reaches this value"),
  warn_cell_v_offset("warn_cell_v_offset", true, 0.1f, 0.1f, 0.00f, 3.0f, "TODO, issue a warning on OLED and serial console if a cell is that close to a OV or UV fault"),
  floor_duty_coolant_pump("floor_duty_coolant_pump", true, 0.25f, 0.25f, 0.00f, 1.0f, "Lowest pump duty cucle when in RUN or CHARGING"),
  cooling_lowt_setpoint("cooling_lowt_setpoint", true, 25.0f, 25.0f, -20.0f, 65.0f, "threashold at wich coolant pump gradually increases duty up to max"),
  cooling_hight_setpoint("cooling_hight_setpoint", true, 35.0f, 35.0f, -20.0f, 65.0f, "threshold at wich coolant pump is at maximum duty"),
  over_t_setpoint("over_t_setpoint", true, 35.0f, 35.0f, 30.0f, 60.0f, "Triggers Over T error; Tesla seam to allow reaching 45C while supercharging; in discharge, 60C is ok"),
  under_t_setpoint("under_t_setpoint", true, -10.0f, -10.0f, -40.0f, 10.0f, "Triggers Under T error"),
  warn_t_offset("warn_t_offset", true, 5.0f, 5.0f, 0.5f, 10.0f, "TODO, issue a warning on OLED and serial console if T is that close to a OT or UT fault"),
  precision_balance_v_setpoint("precision_balance_v_setpoint", true, 4.0f, 4.0f, 3.0f, 4.2f, "start precision balancing when highest cell reaches this setpoint (taken from tom debree)"),
  precision_balance_cell_v_offset("precision_balance_cell_v_offset", true, 0.005f, 0.005f, 0.001f, 0.1f, "precision balance all cells above the lowest cell by this offset (taken from tom debree)"),
  rough_balance_v_setpoint("rough_balance_v_setpoint", true, 3.4f, 3.4f, 3.0f, 4.0f, "start rough balancing when highest cell reaches this setpoint"),
  rough_balance_cell_v_offset("rough_balance_cell_v_offset", true, 0.1f, 0.1f, 0.05f, 0.5f, "rough balance all cells above the lowest cell by this offset"),
  dc2dc_cycle_v_setpoint("dc2dc_cycle_v_setpoint", true, 12.5f, 12.5f, 12.0f, 13.0f, "DC 2 DC 12V battery charging cycle trigger"),
  dc2dc_cycle_time_s("dc2dc_cycle_time_s", true, 3600, 3600, 60, 14400, "DC 2 DC 12V battery charging cycle time in seconds"),
  bat12v_over_v_setpoint("bat12v_over_v_setpoint", true, 14.5f, 14.5f, 13.0f, 15.0f, "Triggers 12V battery OV error"),
  bat12v_under_v_setpoint("bat12v_under_v_setpoint", true, 10.0f, 10.0f, 9.0f, 12.5f, "Triggers 12V battery UV error"),
  bat12v_scaling_divisor("bat12v_scaling_divisor", true, 61.78f, 61.78f, 50.0f, 70.0f, "12V battery ADC devisor 0-1023 -> 0-15V")
{
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
  
}

void Settings::printSettings() {
  Serial.printf("%35s | %-10s | %-10s | [%10s , %-10s] | %s\n", "param name", "value", "default", "min", "max", "description");
  for (auto i = parameters.begin(); i != parameters.end() ; i++) {
    (*i)->prettyPrint();
  }
}
template <>
void ParamImpl<uint32_t>::prettyPrint() {
  Serial.printf("%35s = %-10u | %-10u | [%10u , %-10u] | %s\n", paramName, value, valueDefault, valueMin, valueMax, description);
}
template <>
void ParamImpl<int32_t>::prettyPrint() {
  Serial.printf("%35s = %-10d | %-10d | [%10d , %-10d] | %s\n", paramName, value, valueDefault, valueMin, valueMax, description);
}
template <>
void ParamImpl<float>::prettyPrint() {
  Serial.printf("%35s = %-10.3f | %-10.3f | [%10.3f , %-10.3f] | %s\n", paramName, value, valueDefault, valueMin, valueMax, description);
}

Param * Settings::getParam(const char* name) {
  for (auto i = parameters.begin(); i != parameters.end() ; i++) {
    if (strncmp((*i)->paramName, name, strlen((*i)->paramName)) == 0) {
      return *i;
    }
  }
  return 0;
}
