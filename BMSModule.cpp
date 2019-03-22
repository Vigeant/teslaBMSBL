#include "BMSModule.hpp"
#include "BMSDriver.hpp"
#include "Logger.hpp"

/////////////////////////////////////////////////
/// \brief BMSModule constructor initialized to invalid address 0.
///
/// It's important for the constructor to be without arguments.
/// This allows instantiating all 0x3E modules on the .bss instead of the stack.
/////////////////////////////////////////////////
BMSModule::BMSModule()
{
  resetRecordedValues();
  moduleAddress = 0;
}

/////////////////////////////////////////////////
/// \brief Reset all the values of the object recorded since last reset.
///
/// The prefered way of reseting all values is through a soft board reset using the push button.
/// This function can be used to only reinitialize the values without reseting the whole board.
/////////////////////////////////////////////////
void BMSModule::resetRecordedValues()
{
  for (int i = 0; i < 6; i++)
  {
    cellVolt[i] = 0.0f;
    lowestCellVolt[i] = 5.0f;
    highestCellVolt[i] = 0.0f;
  }
  moduleVolt = 0.0f;
  retmoduleVolt = 0.0f;
  temperatures[0] = 0.0f;
  temperatures[1] = 0.0f;
  lowestTemperature = 200.0f;
  highestTemperature = -100.0f;
  lowestModuleVolt = 200.0f;
  highestModuleVolt = 0.0f;
}

/////////////////////////////////////////////////
/// \brief Balance the cells of the module associated to this object according to the cell mask.
///
/// When balancing is used on a module it will only balance the cells identified by the cellMask for balanceTime seconds.
/// @param cellMask is a byte where the msb = cell0 and bit5 is cell5.
/// @param balanceTime time in seconds (up to 5 seconds) that the cells sheding resistor should be enabled.
/////////////////////////////////////////////////
bool BMSModule::balanceCells(uint8_t cellMask, uint8_t balanceTime) {
  int16_t err;
  //BalanceCells time
  if ((err = BMSDW(moduleAddress, REG_BAL_TIME, balanceTime)) < 0) {
    BMSD_LOG_ERR(moduleAddress, err, "BalanceCells time");
    return false;
  }

  //write balance state to register
  if ((err = BMSDW(moduleAddress, REG_BAL_CTRL, cellMask)) < 0) {
    BMSD_LOG_ERR(moduleAddress, err, "BalanceCells mask");
    return false;
  }
  return true;
}

/////////////////////////////////////////////////
/// \brief This function fetches all the data form the physical tesla module and populates its atributes.
///
/// This function is meant to be called periodically so that the controller can make decision based on the state of the module.
/// The data collected are the faults, the reading of the two temperature sensors and the voltage reading from all 6 cells.
/////////////////////////////////////////////////
bool BMSModule::updateInstanceWithModuleValues()
{
  uint8_t buff[50];
  int16_t err;
  float tempCalc;
  float tempTemp;

  /*
     Status registers
  */
  //BMSDR(moduleAddress, REG_DEV_STATUS, 1, buff);
  //BMSDR(moduleAddress, REG_BAL_TIME, 1, buff);


  if ((err = BMSDR(moduleAddress, REG_ALERT_STATUS, 4, buff)) > 0) {
    alerts = buff[0];
    faults = buff[1];
    COVFaults = buff[2];
    CUVFaults = buff[3];
    LOG_DEBUG("Module %i   alerts=%X   faults=%X   COV=%X   CUV=%X\n", moduleAddress, alerts, faults, COVFaults, CUVFaults);
  } else {
    BMSD_LOG_ERR(moduleAddress, err, "Reading Status Registers");
    return false;
  }
  
  /*
     Voltage and Temperature registers
  */
  //ADC Auto mode, read every ADC input we can (Both Temps, Pack, 6 cells)
  if ((err = BMSDW(moduleAddress, REG_ADC_CTRL, 0b00111101)) < 0) {
    BMSD_LOG_ERR(moduleAddress, err, "ADC Auto mode");
    return false;
  }

  //enable temperature measurement VSS pins
  if ((err = BMSDW(moduleAddress, REG_IO_CTRL, 0b00000011)) < 0) {
    BMSD_LOG_ERR(moduleAddress, err, "enable temperature measurement VSS pins");
    return false;
  }

  //start all ADC conversions
  if ((err = BMSDW(moduleAddress, REG_ADC_CONV, 1)) < 0) {
    BMSD_LOG_ERR(moduleAddress, err, "start all ADC conversions");
    return false;
  }

  //start reading registers at the module voltage registers
  //read 18 bytes (Each value takes 2 - ModuleV, CellV1-6, Temp1, Temp2)
  if ((err = BMSDR(moduleAddress, REG_GPAI, 0x12, buff)) > 0) {
    //payload is 2 bytes gpai, 2 bytes for each of 6 cell voltages, 2 bytes for each of two temperatures (18 bytes of data)
    retmoduleVolt = (buff[0] * 256 + buff[1]) * 0.0020346293922562f;//0.002034609f;
    if (retmoduleVolt > highestModuleVolt) highestModuleVolt = retmoduleVolt;
    if (retmoduleVolt < lowestModuleVolt) lowestModuleVolt = retmoduleVolt;
    for (int i = 0; i < 6; i++)
    {
      cellVolt[i] = (buff[2 + (i * 2)] * 256 + buff[3 + (i * 2)]) * 0.000381493f;
      if (lowestCellVolt[i] > cellVolt[i]) lowestCellVolt[i] = cellVolt[i];
      if (highestCellVolt[i] < cellVolt[i]) highestCellVolt[i] = cellVolt[i];
    }
    //use added up cells and not reported module voltage
    moduleVolt = 0;
    for (int i = 0; i < 6; i++)
    {
      moduleVolt = moduleVolt + cellVolt[i];
    }
  } else {
    BMSD_LOG_ERR(moduleAddress, err, "Reading voltage registers");
    return false;
  }
  
  //Now using steinhart/hart equation for temperatures. We'll see if it is better than old code.
  tempTemp = (1.78f / ((buff[14] * 256 + buff[15] + 2) / 33046.0f) - 3.57f);
  tempTemp *= 1000.0f;
  tempCalc =  1.0f / (0.0007610373573f + (0.0002728524832 * logf(tempTemp)) + (powf(logf(tempTemp), 3) * 0.0000001022822735f));
  temperatures[0] = tempCalc - 273.15f;

  tempTemp = 1.78f / ((buff[16] * 256 + buff[17] + 9) / 33068.0f) - 3.57f;
  tempTemp *= 1000.0f;
  tempCalc = 1.0f / (0.0007610373573f + (0.0002728524832 * logf(tempTemp)) + (powf(logf(tempTemp), 3) * 0.0000001022822735f));
  temperatures[1] = tempCalc - 273.15f;

  if (getLowTemp() < lowestTemperature) lowestTemperature = getLowTemp();
  if (getHighTemp() > highestTemperature) highestTemperature = getHighTemp();

  LOG_DEBUG("Got voltage and temperature readings\n");
  return true;
}

/////////////////////////////////////////////////
/// \brief returns the faults registered in this module.
///
/// | bool mask | hex mask | fault description |
/// |:---------:|:--------:|-------------------|
/// |0b00000001 | 0x01 | Cell Over Voltage, check COVFaults to identify affected cells.|
/// |0b00000010 | 0x02 | Cell Under Voltage, check CUVFaults to identify affected cells.|
/// |0b00000100 | 0x04 | CRC error in received packet.|
/// |0b00001000 | 0x08 | Power on reset has occurred.|
/// |0b00010000 | 0x10 | Test fault active.|
/// |0b00100000 | 0x20 | Internal registers inconsistent.|
/////////////////////////////////////////////////
uint8_t BMSModule::getFaults()
{
  return faults;
}

/////////////////////////////////////////////////
/// \brief returns the alerts registered in this module.
///
/// | bool mask | hex mask | Alert description |
/// |:---------:|:--------:|-------------------|
/// |0b00000001 | 0x01 | Over temperature on TS1.|
/// |0b00000010 | 0x02 | Over temperature on TS2.|
/// |0b00000100 | 0x04 | Sleep mode active.|
/// |0b00001000 | 0x08 | Thermal shutdown active.|
/// |0b00010000 | 0x10 | Test Alert.|
/// |0b00100000 | 0x20 | OTP EPROM Uncorrectable Error.|
/// |0b01000000 | 0x40 | GROUP3 Regs Invalid.|
/// |0b10000000 | 0x80 | Address not registered.|
/////////////////////////////////////////////////
uint8_t BMSModule::getAlerts()
{
  return alerts;
}

/////////////////////////////////////////////////
/// \brief returns a bit map of the OV cells.
//////////////////////////////////////////////////
uint8_t BMSModule::getCOVCells()
{
  return COVFaults;
}

/////////////////////////////////////////////////
/// \brief returns a bit map of the UV cells.
//////////////////////////////////////////////////
uint8_t BMSModule::getCUVCells()
{
  return CUVFaults;
}

/////////////////////////////////////////////////
/// \brief returns the voltage of a cell.
///
/// @param cell The cell index
//////////////////////////////////////////////////
float BMSModule::getCellVoltage(int cell)
{
  if (cell < 0 || cell > 5) return 0.0f;
  return cellVolt[cell];
}

/////////////////////////////////////////////////
/// \brief returns the voltage of the lowest voltage cell.
//////////////////////////////////////////////////
float BMSModule::getLowCellV()
{
  float lowVal = 10.0f;
  for (int i = 0; i < 6; i++) if (cellVolt[i] < lowVal) lowVal = cellVolt[i];
  return lowVal;
}

/////////////////////////////////////////////////
/// \brief returns the voltage of the highest voltage cell.
//////////////////////////////////////////////////
float BMSModule::getHighCellV()
{
  float hiVal = 0.0f;
  for (int i = 0; i < 6; i++) if (cellVolt[i] > hiVal) hiVal = cellVolt[i];
  return hiVal;
}

/////////////////////////////////////////////////
/// \brief returns the average voltage of all 6 cells within this module.
//////////////////////////////////////////////////
float BMSModule::getAverageV()
{
  int x = 0;
  float avgVal = 0.0f;
  for (int i = 0; i < 6; i++)
  {
    if (cellVolt[i] < 60.0)
    {
      x++;
      avgVal += cellVolt[i];
    }
  }

  avgVal /= x;
  return avgVal;
}

/////////////////////////////////////////////////
/// \brief returns the highest voltage reached by a cell since last reset of the atributes.
//////////////////////////////////////////////////
float BMSModule::getHighestModuleVolt()
{
  return highestModuleVolt;
}

/////////////////////////////////////////////////
/// \brief returns the lowest voltage reached by a cell since last reset of the atributes.
//////////////////////////////////////////////////
float BMSModule::getLowestModuleVolt()
{
  return lowestModuleVolt;
}

/////////////////////////////////////////////////
/// \brief returns the highest voltage reached by each individual cells since last reset of the atributes.
//////////////////////////////////////////////////
float BMSModule::getHighestCellVolt(int cell)
{
  if (cell < 0 || cell > 5) return 0.0f;
  return highestCellVolt[cell];
}

/////////////////////////////////////////////////
/// \brief returns the lowest voltage reached by a cell since last reset of the atributes.
//////////////////////////////////////////////////
float BMSModule::getLowestCellVolt(int cell)
{
  if (cell < 0 || cell > 5) return 0.0f;
  return lowestCellVolt[cell];
}

/////////////////////////////////////////////////
/// \brief returns the highest temperature reached by the module since last reset of the atributes.
//////////////////////////////////////////////////
float BMSModule::getHighestTemp()
{
  return highestTemperature;
}

/////////////////////////////////////////////////
/// \brief returns the lowest temperature reached by the module since last reset of the atributes.
//////////////////////////////////////////////////
float BMSModule::getLowestTemp()
{
  return lowestTemperature;
}

/////////////////////////////////////////////////
/// \brief returns the lower temperature of the two temperature sensors.
//////////////////////////////////////////////////
float BMSModule::getLowTemp()
{
  return (temperatures[0] < temperatures[1]) ? temperatures[0] : temperatures[1];
}

/////////////////////////////////////////////////
/// \brief returns the higher temperature of the two temperature sensors.
//////////////////////////////////////////////////
float BMSModule::getHighTemp()
{
  return (temperatures[0] < temperatures[1]) ? temperatures[1] : temperatures[0];
}

/////////////////////////////////////////////////
/// \brief returns the average temperature of the two temperature sensors.
//////////////////////////////////////////////////
float BMSModule::getAvgTemp()
{
  return (temperatures[0] + temperatures[1]) / 2.0f;
}

/////////////////////////////////////////////////
/// \brief returns the module voltage.
//////////////////////////////////////////////////
float BMSModule::getModuleVoltage()
{
  return moduleVolt;
}

/////////////////////////////////////////////////
/// \brief returns the temperature reading of a temperature sensor.
///
/// @param temp temp sensor index (0 or 1)
//////////////////////////////////////////////////
float BMSModule::getTemperature(int temp)
{
  if (temp < 0 || temp > 1) return 0.0f;
  return temperatures[temp];
}

/////////////////////////////////////////////////
/// \brief Sets the address of the module associated to this object instance.
///
/// @param newAddr The address to assign to this module (0 - 0x3e). 0 would mean the module is not used.
//////////////////////////////////////////////////
void BMSModule::setAddress(uint8_t newAddr)
{
  if (newAddr < 0 || newAddr > MAX_MODULE_ADDR) return;
  moduleAddress = newAddr;
}

/////////////////////////////////////////////////
/// \brief Returns the address of the module associated to this object instance.
//////////////////////////////////////////////////
uint8_t BMSModule::getAddress()
{
  return moduleAddress;
}
