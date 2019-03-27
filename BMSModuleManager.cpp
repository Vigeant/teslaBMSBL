#include "CONFIG.h"
#include "BMSModuleManager.hpp"
#include "Logger.hpp"

/////////////////////////////////////////////////
/// \brief constructor initialized to invalid address 0.
/////////////////////////////////////////////////
BMSModuleManager::BMSModuleManager()
{
  histLowestPackVolt = 1000.0f;
  histHighestPackVolt = 0.0f;
  histLowestPackTemp = 200.0f;
  histHighestPackTemp = -100.0f;
  histLowestCellVolt = 5.0f;
  histHighestCellVolt = 0.0f;
  histHighestCellDiffVolt = 0.0f;
  lineFault = false;
  pstring = 1;
}

/////////////////////////////////////////////////
/// \brief resets all the modules atributes to their initial value.
/////////////////////////////////////////////////
void BMSModuleManager::resetModuleRecordedValues()
{
  for (int y = 0; y < MAX_MODULE_ADDR; y++)
  {
    modules[y].resetRecordedValues();
  }
}

/////////////////////////////////////////////////
/// \brief perform a round of balancing.
///
/// @param duration the number of seconds to enable balancing for.
/////////////////////////////////////////////////
void BMSModuleManager::balanceCells(uint8_t duration)
{
  uint8_t balance = 0;//bit 0 - 5 are to activate cell balancing 1-6

  for (int y = 0; y < MAX_MODULE_ADDR; y++)
  {
    if (modules[y].getAddress() > 0)
    {
      balance = 0;
      for (int i = 0; i < 6; i++)
      {
        if (modules[y].getCellVoltage(i) > getLowCellVolt() + BALANCE_CELL_V_OFFSET)
        {
          balance = balance | (1 << i);

        }
      }
      LOG_DEBUG("balancing module %d - 0x%x\n", modules[y].getAddress(), balance);
      (void) modules[y].balanceCells(balance, duration);
    } else {
      //no more modules
      break;
    }
  }
}

/////////////////////////////////////////////////
/// \brief reset board addresses to a sequence from closest to BMS to farthest.
///
/// Force all modules to reset back to address 0 then set them all up in order so that the first module
/// in line from the master board is 1, the second one 2, and so on.
/////////////////////////////////////////////////
void BMSModuleManager::renumberBoardIDs()
{
  int16_t err;
  uint8_t buff[30];

  //Reset addresses to 0 in objects
  for (int y = 0; y < MAX_MODULE_ADDR; y++) {
    modules[y].setAddress(0);
  }

  //Reset addresses to 0 in boards
  int tempNumFoundModules = 0;
  LOG_INFO("\n\nReseting all boards\n\n");
  if ((err = BMSDW(BROADCAST_ADDR, 0x3C, 0xA5)) < 0) {
    BMSD_LOG_ERR(BROADCAST_ADDR, err, "Broadcasting reset");
  }

  //assign address to boards that respond to address 0
  for (int y = 0; y < MAX_MODULE_ADDR; y++) {
    LOG_INFO("sending read on address 0\n");

    //check if a board responds to address 0
    if ((err = BMSDR(0, 0, 1, buff)) != READ_CRC_FAIL) {
      if (err == READ_RECV_LEN_MISMATCH) {
        LOG_INFO("Did not get a response on address 0... done assigning addresses\n", y);
        break;
      } else if (err < 0) {
        //retry
        y--;
        continue;
      }

    }
    LOG_INFO("Got a response to address 0\n");

    //write address register
    LOG_INFO("Assigning it address 0x%x\n", y + 1);
    if ((err = BMSDW(0, REG_ADDR_CTRL, (y + 1) | 0x80)) < 0 ) {
      BMSD_LOG_ERR(y + 1, err, "write address register");
    }
    modules[y].setAddress(y + 1);
    LOG_INFO("Address %d assigned\n", modules[y].getAddress());
    tempNumFoundModules++;
  }
  numFoundModules = tempNumFoundModules;
}

/////////////////////////////////////////////////
/// \brief clear board faults and alerts.
///
/// After a RESET boards have their faults written due to the hard restart
/// or first time power up, this clears their faults
/////////////////////////////////////////////////
void BMSModuleManager::clearFaults()
{
  int16_t err;
  //reset alerts status
  if ((err = BMSDW(BROADCAST_ADDR, REG_ALERT_STATUS, 0xFF)) < 0) {
    BMSD_LOG_ERR(BROADCAST_ADDR, err, "reset alerts status");
  }

  //clear alerts status
  if ((err = BMSDW(BROADCAST_ADDR, REG_ALERT_STATUS, 0)) < 0) {
    BMSD_LOG_ERR(BROADCAST_ADDR, err, "clear alerts status");
  }

  //reset faults status
  if ((err = BMSDW(BROADCAST_ADDR, REG_FAULT_STATUS, 0xFF)) < 0) {
    BMSD_LOG_ERR(BROADCAST_ADDR, err, "reset faults status");
  }

  //clear faults status
  if ((err = BMSDW(BROADCAST_ADDR, REG_FAULT_STATUS, 0)) < 0) {
    BMSD_LOG_ERR(BROADCAST_ADDR, err, "clear faults status");
  }
}

/////////////////////////////////////////////////
/// \brief puts boards to sleep to save power.
///
/// Puts all boards on the bus into a Sleep state, very good to use when the vehicle is at rest state.
/// Pulling the boards out of sleep only to check voltage decay and temperature when the contactors are open.
/////////////////////////////////////////////////
void BMSModuleManager::sleepBoards() {
  int16_t err;
  if ((err = BMSDW(BROADCAST_ADDR, REG_IO_CTRL, 0x04)) < 0) {
    BMSD_LOG_ERR(BROADCAST_ADDR, err, "put boards to sleep");
  }
}

/////////////////////////////////////////////////
/// \brief wake boards.
///
/// Wakes all the boards up and clears their SLEEP state bit in the Alert Status Registery
/////////////////////////////////////////////////
void BMSModuleManager::wakeBoards()
{
  int16_t err;
  //wake boards up
  if ((err = BMSDW(BROADCAST_ADDR, REG_IO_CTRL, 0x00)) < 0) {
    BMSD_LOG_ERR(BROADCAST_ADDR, err, "wake boards up");
  }

  //reset faults
  if ((err = BMSDW(BROADCAST_ADDR, REG_ALERT_STATUS, 0x04)) < 0) {
    BMSD_LOG_ERR(BROADCAST_ADDR, err, "wake boards up reset faults");
  }

  //clear faults
  if ((err = BMSDW(BROADCAST_ADDR, REG_ALERT_STATUS, 0x00)) < 0) {
    BMSD_LOG_ERR(BROADCAST_ADDR, err, "wake boards up clear faults");
  }
}

/////////////////////////////////////////////////
/// \brief This function synchronises each module instance with its physical board.
///
/// Most important function called every tick
/////////////////////////////////////////////////
void BMSModuleManager::getAllVoltTemp() {
  int16_t err;
  float tempPackVolt = 0.0f;
  if (lineFault || modules[0].getAddress() == 0) renumberBoardIDs();

  //stop balancing
  if ((err = BMSDW(BROADCAST_ADDR, REG_BAL_CTRL, 0x00)) < 0) {
    BMSD_LOG_ERR(BROADCAST_ADDR, err, "getAllVoltTemp, stop balancing");
    lineFault = true;
  } else {
    lineFault = false;
  }

  //update state of each module and gather voltages and temperatures
  for (int y = 0; y < MAX_MODULE_ADDR; y++)
  {
    if (modules[y].getAddress() > 0) {
      modules[y].updateInstanceWithModuleValues();
      tempPackVolt += modules[y].getModuleVoltage();
      if (modules[y].getLowTemp() < histLowestPackTemp) histLowestPackTemp = modules[y].getLowTemp();
      if (modules[y].getHighTemp() > histHighestPackTemp) histHighestPackTemp = modules[y].getHighTemp();
    } else {
      break;
    }
  }

  //update high and low watermark values for voltages
  tempPackVolt = tempPackVolt / pstring;
  if (tempPackVolt > histHighestPackVolt) histHighestPackVolt = tempPackVolt;
  if (tempPackVolt < histLowestPackVolt) histLowestPackVolt = tempPackVolt;

  float tempHighCellVolt = 0.0;
  for (int y = 0; y < MAX_MODULE_ADDR; y++)
  {
    if (modules[y].getAddress() > 0) {
      if (modules[y].getHighCellV() >  tempHighCellVolt)  tempHighCellVolt = modules[y].getHighCellV();
    }
  }
  float tempLowCellVolt = 5.0;
  for (int y = 0; y < MAX_MODULE_ADDR; y++)
  {
    if (modules[y].getAddress() > 0) {
      if (modules[y].getLowCellV() <  tempLowCellVolt)  tempLowCellVolt = modules[y].getLowCellV();
    }
  }

  //update cell V watermarks
  if ( tempLowCellVolt < histLowestCellVolt ) histLowestCellVolt =  tempLowCellVolt;
  if ( tempHighCellVolt > histHighestCellVolt ) histHighestCellVolt = tempHighCellVolt;

  float tempHCDV = tempHighCellVolt - tempLowCellVolt;
  if ( histHighestCellDiffVolt < tempHCDV ) histHighestCellDiffVolt = tempHCDV;

  //save values to objects
  lowCellVolt = tempLowCellVolt;
  highCellVolt = tempHighCellVolt;
  packVolt = tempPackVolt;
}

/////////////////////////////////////////////////
/// \brief returns the highest temperature reached by the pack since last reset of the attributes.
//////////////////////////////////////////////////
float BMSModuleManager::getHistHighestPackTemp() {
  return histHighestPackTemp;
}

/////////////////////////////////////////////////
/// \brief returns the highest voltage difference between 2 cells reached by the pack since last reset of the attributes.
//////////////////////////////////////////////////
float BMSModuleManager::getHistHighestCellDiffVolt() {
  return histHighestCellDiffVolt;
}

/////////////////////////////////////////////////
/// \brief returns voltage of the lowest cell
//////////////////////////////////////////////////
float BMSModuleManager::getLowCellVolt() {
  return lowCellVolt;
}

/////////////////////////////////////////////////
/// \brief returns voltage of the highest cell
//////////////////////////////////////////////////
float BMSModuleManager::getHighCellVolt() {
  return highCellVolt;
}

/////////////////////////////////////////////////
/// \brief returns total pack voltage
//////////////////////////////////////////////////
float BMSModuleManager::getPackVoltage() {
  return packVolt;
}

/////////////////////////////////////////////////
/// \brief returns the lowest cell voltage reached within the pack since last reset of the attributes.
//////////////////////////////////////////////////
float BMSModuleManager::getHistLowestCellVolt() {
  return histLowestCellVolt;
}

/////////////////////////////////////////////////
/// \brief returns the highest cell voltage reached within the pack since last reset of the atributes.
//////////////////////////////////////////////////
float BMSModuleManager::getHistHighestCellVolt() {
  return histHighestCellVolt;
}

/////////////////////////////////////////////////
/// \brief returns the lowest pack voltage reached since last reset of the atributes.
//////////////////////////////////////////////////
float BMSModuleManager::getHistLowestPackVolt() {
  return histLowestPackVolt;
}

/////////////////////////////////////////////////
/// \brief returns the highest pack voltage reached since last reset of the atributes.
//////////////////////////////////////////////////
float BMSModuleManager::getHistHighestPackVolt() {
  return histHighestPackVolt;
}

/////////////////////////////////////////////////
/// \brief not used
//////////////////////////////////////////////////
void BMSModuleManager::setBatteryID(int id)
{
  batteryID = id;
}

/////////////////////////////////////////////////
/// \brief not used
//////////////////////////////////////////////////
void BMSModuleManager::setPstrings(int pstrings)
{
  pstring = pstrings;
}

/////////////////////////////////////////////////
/// \brief returns the average temperature of the pack.
//////////////////////////////////////////////////
float BMSModuleManager::getAvgTemperature()
{
  float avg = 0.0f;
  highTemp = -100;
  lowTemp = 999;
  int y = 0; //counter for modules above -70 (sensors connected)
  for (int x = 0; x < MAX_MODULE_ADDR; x++)
  {
    if (modules[x].getAddress() > 0) {
      if (modules[x].getAvgTemp() > -70) {
        avg += modules[x].getAvgTemp();
        y++;
      }
    } else {
      break;
    }
  }
  if (y > 0) {
    return avg / (float)(y);
  } else {
    return 0;
  }
}

/////////////////////////////////////////////////
/// \brief returns the current highest temperature of the pack.
//////////////////////////////////////////////////
float BMSModuleManager::getHighTemperature()
{
  highTemp = -100;
  for (int x = 0; x < MAX_MODULE_ADDR; x++) {
    if (modules[x].getAddress() > 0) {
      if (modules[x].getAvgTemp() > -70) {
        if (modules[x].getAvgTemp() > highTemp) {
          highTemp = modules[x].getAvgTemp();
        }
      }
    } else {
      break;
    }
  }
  return highTemp;
}

/////////////////////////////////////////////////
/// \brief returns the current lowest temperature of the pack.
//////////////////////////////////////////////////
float BMSModuleManager::getLowTemperature()
{
  lowTemp = 999;
  for (int x = 0; x < MAX_MODULE_ADDR; x++) {
    if (modules[x].getAddress() > 0) {
      if (modules[x].getAvgTemp() > -70) {
        if (modules[x].getAvgTemp() < lowTemp) {
          lowTemp = modules[x].getAvgTemp();
        }
      }
    } else {
      break;
    }
  }
  return lowTemp;
}

/////////////////////////////////////////////////
/// \brief returns the current average cell voltage for the whole pack.
//////////////////////////////////////////////////
float BMSModuleManager::getAvgCellVolt()
{
  float avg = 0.0f;
  for (int x = 0; x < MAX_MODULE_ADDR; x++)
  {
    if (modules[x].getAddress() > 0) avg += modules[x].getAverageV();
  }
  avg = avg / (float)numFoundModules;

  return avg;
}

/////////////////////////////////////////////////
/// \brief returns true if the serial communication is broken.
//////////////////////////////////////////////////
bool BMSModuleManager::getLineFault() {
  return lineFault;
}

/////////////////////////////////////////////////
/// \brief prints the pack summary to the console.
//////////////////////////////////////////////////
void BMSModuleManager::printPackSummary()
{
  uint8_t faults;
  uint8_t alerts;
  uint8_t COV;
  uint8_t CUV;

  LOG_CONSOLE("\nModules: %i    Voltage: %.2fV   Avg Cell Voltage: %.2fV     Avg Temp: %.2fC\n", numFoundModules,
              getPackVoltage(), getAvgCellVolt(), getAvgTemperature());
  for (int y = 0; y < MAX_MODULE_ADDR; y++)
  {
    if (modules[y].getAddress() > 0)
    {
      faults = modules[y].getFaults();
      alerts = modules[y].getAlerts();
      COV = modules[y].getCOVCells();
      CUV = modules[y].getCUVCells();
      LOG_CONSOLE("\n=================================================================\n");
      LOG_CONSOLE("=                              Module #%2i                      =\n", y);
      LOG_CONSOLE("=================================================================\n");
      LOG_CONSOLE("\t============================== Cell details =====================\n");

      LOG_CONSOLE("\tVoltage: %3.2fV (%3.2fV-%.2fV)\tTemperatures: (%3.2fC-%3.2fC)\n", modules[y].getModuleVoltage(),
                  modules[y].getLowCellV(), modules[y].getHighCellV(), modules[y].getLowTemp(), modules[y].getHighTemp());
      LOG_CONSOLE("\tHistoric Voltages: (%3.2fV-%.2fV)\tTemperatures: (%3.2fC-%3.2fC)\n", modules[y].getLowestModuleVolt(),
                  modules[y].getHighestModuleVolt(), modules[y].getLowestTemp(), modules[y].getHighestTemp());
                  
      for (int i = 0; i < 6; i++) {
        LOG_CONSOLE("\tCell%2d: %3.2fV | historic lowest: %3.2fV | historic highest: %3.2fV\n", i + 1, modules[y].getCellVoltage(i), modules[y].getLowestCellVolt(i), modules[y].getHighestCellVolt(i));
      }

      if (faults > 0) {
        LOG_CONSOLE("  MODULE IS FAULTED:\n");
        if (faults & 1) {
          LOG_CONSOLE("    Overvoltage Cell Numbers (1-6): ");
          for (int i = 0; i < 6; i++) {
            if (COV & (1 << i)) {
              LOG_CONSOLE("%d ", i + 1);
            }
          }
          LOG_CONSOLE("\n");
        }
        if (faults & 2) {
          LOG_CONSOLE("    Undervoltage Cell Numbers (1-6): ");
          for (int i = 0; i < 6; i++) {
            if (CUV & (1 << i)) {
              LOG_CONSOLE("%d ", i + 1);
            }
          }
          LOG_CONSOLE("\n");
        }
        if (faults & 4) {
          LOG_CONSOLE("    CRC error in received packet\n");
        }
        if (faults & 8) {
          LOG_CONSOLE("    Power on reset has occurred\n");
        }
        if (faults & 0x10) {
          LOG_CONSOLE("    Test fault active\n");
        }
        if (faults & 0x20) {
          LOG_CONSOLE("    Internal registers inconsistent\n");
        }
      }
      if (alerts > 0) {
        LOG_CONSOLE("  MODULE HAS ALERTS:\n");
        if (alerts & 1) {
          LOG_CONSOLE("    Over temperature on TS1\n");
        }
        if (alerts & 2) {
          LOG_CONSOLE("    Over temperature on TS2\n");
        }
        if (alerts & 4) {
          LOG_CONSOLE("    Sleep mode active\n");
        }
        if (alerts & 8) {
          LOG_CONSOLE("    Thermal shutdown active\n");
        }
        if (alerts & 0x10) {
          LOG_CONSOLE("    Test Alert\n");
        }
        if (alerts & 0x20) {
          LOG_CONSOLE("    OTP EPROM Uncorrectable Error\n");
        }
        if (alerts & 0x40) {
          LOG_CONSOLE("    GROUP3 Regs Invalid\n");
        }
        if (alerts & 0x80) {
          LOG_CONSOLE("    Address not registered\n");
        }
      }
      if (faults > 0 || alerts > 0) LOG_CONSOLE("\n");
    }
  }
}

/////////////////////////////////////////////////
/// \brief prints the pack details to the console.
//////////////////////////////////////////////////
void BMSModuleManager::printPackDetails()
{
  //uint8_t faults;
  //uint8_t alerts;
  //uint8_t COV;
  //uint8_t CUV;
  int cellNum = 0;

  LOG_CONSOLE("\nModules: %i  Strings: %i  Voltage: %.2fV   Avg Cell Voltage: %.2fV  Low Cell Voltage: %.2fV   High Cell Voltage: %.2fV\n", numFoundModules,
              pstring, getPackVoltage(), getAvgCellVolt(), lowCellVolt, highCellVolt );
  LOG_CONSOLE("Delta Voltage: %.3fV   Avg Temp: %.2fC \n", (highCellVolt - lowCellVolt), getAvgTemperature());
  LOG_CONSOLE("\n");
  for (int y = 0; y < MAX_MODULE_ADDR; y++)
  {
    if (modules[y].getAddress() > 0)
    {
      //faults = modules[y].getFaults();
      //alerts = modules[y].getAlerts();
      //COV = modules[y].getCOVCells();
      //CUV = modules[y].getCUVCells();

      LOG_CONSOLE("Module #%d\n", modules[y].getAddress());
      if (y < 10) LOG_CONSOLE(" ");
      LOG_CONSOLE("  %.2fV\n", modules[y].getModuleVoltage());
      for (int i = 0; i < 6; i++)
      {
        if (cellNum < 10) LOG_CONSOLE(" ");
        LOG_CONSOLE("  Cell%d: %.2fV\n", cellNum++, modules[y].getCellVoltage(i));
      }
      LOG_CONSOLE("  Neg Term Temp: %.2fC\t", modules[y].getTemperature(0));
      LOG_CONSOLE("Pos Term Temp: %.2fC\n", modules[y].getTemperature(1));
    }
  }
}

/////////////////////////////////////////////////
/// \brief prints the pack details in CSV format to the console.
//////////////////////////////////////////////////
void BMSModuleManager::printAllCSV()
{
  for (int y = 0; y < MAX_MODULE_ADDR; y++)
  {
    if (modules[y].getAddress() > 0)
    {
      LOG_CONSOLE("%d", modules[y].getAddress());
      LOG_CONSOLE(",");
      for (int i = 0; i < 6; i++)
      {
        LOG_CONSOLE("%.2f,", modules[y].getCellVoltage(i));
      }
      LOG_CONSOLE("%.2f,", modules[y].getTemperature(0));
      LOG_CONSOLE("%.2f\n", modules[y].getTemperature(1));
    }
  }
}
