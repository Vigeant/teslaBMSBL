#include "Config.hpp"
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
void BMSModuleManager::balanceCells(uint8_t duration, float cell_v_offset)
{
  uint8_t balance = 0;//bit 0 - 5 are to activate cell balancing 1-6

  for (int y = 0; y < MAX_MODULE_ADDR; y++)
  {
    if (modules[y].getAddress() > 0)
    {
      balance = 0;
      for (int i = 0; i < 6; i++)
      {
        if (modules[y].getCellVoltage(i) > getLowCellVolt() + cell_v_offset)
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
  //if ((err = BMSDW(BROADCAST_ADDR, REG_BAL_CTRL, 0x3f)) < 0) {
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
  if (TESTING_MODE == 1) tempLowCellVolt = 3.8;
  
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

  for (int y = 0; y < MAX_MODULE_ADDR; y++)
  {
    if (modules[y].getAddress() > 0)
    {
      faults = modules[y].getFaults();
      alerts = modules[y].getAlerts();
      COV = modules[y].getCOVCells();
      CUV = modules[y].getCUVCells();
      LOG_CONSOLE("\n=====================================================================\n");
      LOG_CONSOLE("=                                Module #%2i                         =\n", y+1);
      LOG_CONSOLE("=====================================================================\n");
      //LOG_CONSOLE("\t============================== Cell details =====================\n");

      LOG_CONSOLE("Voltage: %3.2fV (%3.2fV-%.2fV)\t\tTemperatures: (%3.2fC-%3.2fC)\n", modules[y].getModuleVoltage(),
                  modules[y].getLowCellV(), modules[y].getHighCellV(), modules[y].getLowTemp(), modules[y].getHighTemp());
      LOG_CONSOLE("Historic Voltages: (%3.2fV-%.2fV)\tTemperatures: (%3.2fC-%3.2fC)\n", modules[y].getLowestModuleVolt(),
                  modules[y].getHighestModuleVolt(), modules[y].getLowestTemp(), modules[y].getHighestTemp());
      LOG_CONSOLE("+------+---------+---------+----------+\n");
      LOG_CONSOLE("|Cell #| Cell V  |lowest V |highest V |\n");
      LOG_CONSOLE("+------+---------+---------+----------+\n");
      for (int i = 0; i < 6; i++) {
        LOG_CONSOLE("|  %2d  |  %.3f  |  %.3f  |  %.3f   |\n", i + 1, modules[y].getCellVoltage(i), modules[y].getLowestCellVolt(i), modules[y].getHighestCellVolt(i));
      }
      LOG_CONSOLE("+------+---------+---------+----------+\n");

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
  LOG_CONSOLE("\n=====================================================================\n");
  LOG_CONSOLE("\nModules: %i    Voltage: %.2fV   Avg Cell Voltage: %.2fV     Avg Temp: %.2fC\n", numFoundModules,
              getPackVoltage(), getAvgCellVolt(), getAvgTemperature());
  LOG_CONSOLE("INL_EVSE_DISC: %d\n", digitalRead(INL_EVSE_DISC));
  LOG_CONSOLE("INH_RUN: %d\n", digitalRead(INH_RUN));
  LOG_CONSOLE("INH_CHARGING: %d\n", digitalRead(INH_CHARGING));
  
  //testing scafolding
  LOG_CONSOLE("getHighCellVolt() < CHARGER_CYCLE_V_SETPOINT    : %f < %f?\n" , getHighCellVolt(),CHARGER_CYCLE_V_SETPOINT);
  LOG_CONSOLE("getHighCellVolt() < MAX_CHARGE_V_SETPOINT    : %f < %f?\n" , getHighCellVolt(),MAX_CHARGE_V_SETPOINT);
}

/////////////////////////////////////////////////
/// \brief prints the pack details to the console.
//////////////////////////////////////////////////
void BMSModuleManager::printPackGraph()
{
  char graphLine[200];
  int cellX, coli;
  float deltaV =  highCellVolt - lowCellVolt;
  float rowV;
  char barchar = 'Z';
  unsigned int seconds = millis()/1000;   
  
  memset(graphLine, 0, 86);
  LOG_CONSOLE("\n====================================================================================\n");
  
  LOG_CONSOLE("=  %3d days, %02d:%02d:%02d            cell voltage Graph (V)                            =\n",
  seconds/86400, (seconds % 86400)/3600, (seconds % 3600)/60, (seconds % 60));
  LOG_CONSOLE("====================================================================================\n");


  //print graph header
  LOG_CONSOLE("          ");
  for (int mod = 0; mod < numFoundModules; mod ++){
    LOG_CONSOLE(" | M%-2d|" , mod + 1);
  }
  LOG_CONSOLE("\n");

  LOG_CONSOLE("          ");
  for (cellX = 0; cellX < numFoundModules * 6; cellX++){
    if (cellX % 6 == 0){
      LOG_CONSOLE(" ");
    }
    LOG_CONSOLE("%d", cellX%6 + 1);
  }
  LOG_CONSOLE("\n");
  
  LOG_CONSOLE("          ");
  for (cellX = 0; cellX < numFoundModules * 7; cellX++){
    graphLine[cellX] = '=';
  }
  graphLine[cellX] = '\n';
  LOG_CONSOLE(graphLine);

  /*for (int row = 128; row <= 256 ; row++){
    LOG_CONSOLE("%i : %c ", row, row);
  }*/
  
  for (int row = 40; row >= 0 ; row--){
    memset(graphLine, 0, 86);
    rowV = deltaV*row/40 + lowCellVolt;
    LOG_CONSOLE("%.3fV |" , rowV);

    if (getHighCellVolt() > PRECISION_BALANCE_V_SETPOINT){
      if(rowV > getLowCellVolt() + PRECISION_BALANCE_CELL_V_OFFSET){
        LOG_CONSOLE("B ");
        barchar = 'B';
      } else{
        LOG_CONSOLE("| ");
        barchar = 177;
      }
    } else if (getHighCellVolt() > ROUGH_BALANCE_V_SETPOINT){
      if(rowV > getLowCellVolt() + ROUGH_BALANCE_CELL_V_OFFSET){
        LOG_CONSOLE("B ");
        barchar = 'B';
      } else{
        LOG_CONSOLE("| ");
        barchar = 177;
      }
    } else {
      LOG_CONSOLE("| ");
    }
    for (cellX = 0, coli = 0; cellX < numFoundModules * 6; cellX++,coli++){
      if (cellX % 6 == 0){
        graphLine[coli] = '|';
        coli ++;
      }
      if (modules[cellX/6].getCellVoltage(cellX%6) < rowV){
        graphLine[coli] = ' ';
      } else {
        graphLine[coli] = barchar;
        //graphLine[cellX] = 'X';
      }
    }
    graphLine[coli] = '\n';
    LOG_CONSOLE(graphLine);
  }

  LOG_CONSOLE("          ");
  for (cellX = 0; cellX < numFoundModules * 7; cellX++){
    graphLine[cellX] = '=';
  }
  graphLine[cellX] = '\n';
  LOG_CONSOLE(graphLine);
  LOG_CONSOLE("          ");
  for (cellX = 0; cellX < numFoundModules * 6; cellX++){
    if (cellX % 6 == 0){
      LOG_CONSOLE(" ");
    }
    LOG_CONSOLE("%d", cellX%6 + 1);
  }
  LOG_CONSOLE("\n          ");
  for (int mod = 0; mod < numFoundModules; mod ++){
    LOG_CONSOLE(" | M%-2d|" , mod + 1);
  }
  LOG_CONSOLE("\n");
}

/////////////////////////////////////////////////
/// \brief prints the pack details in CSV format to the console.
//////////////////////////////////////////////////
void BMSModuleManager::printAllCSV()
{
  LOG_CONSOLE("Module#,time (ms),cell1,cell2,cell3,cell4,cell5,cell6,temp1,temp2\n");
  for (int y = 0; y < MAX_MODULE_ADDR; y++)
  {
    if (modules[y].getAddress() > 0)
    {
      LOG_CONSOLE("%d", modules[y].getAddress());
      LOG_CONSOLE(",");
      LOG_CONSOLE("%d", millis());
      LOG_CONSOLE(",");
      for (int i = 0; i < 6; i++)
      {
        LOG_CONSOLE("%.3f,", modules[y].getCellVoltage(i));
      }
      LOG_CONSOLE("%.2f,", modules[y].getTemperature(0));
      LOG_CONSOLE("%.2f\n", modules[y].getTemperature(1));
    }
  }
}
