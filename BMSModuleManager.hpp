#ifndef BMSMODULEMANAGER_HPP_
#define BMSMODULEMANAGER_HPP_
#include <Arduino.h>
#include "BMSModule.hpp"
#include "BMSDriver.hpp"

class BMSModuleManager
{
  public:
    BMSModuleManager();
    int seriescells();
    void resetModuleRecordedValues();
    void StopBalancing();
    void balanceCells(uint8_t duration);
    void renumberBoardIDs();
    void clearFaults();
    void sleepBoards();
    void wakeBoards();
    void getAllVoltTemp();
    void readSetpoints();
    void setBatteryID(int id);
    void setPstrings(int Pstrings);
    void setUnderVolt(float newVal);
    void setOverVolt(float newVal);
    void setOverTemp(float newVal);
    void setBalanceV(float newVal);
    void setBalanceHyst(float newVal);
    void setSensors(int sensor, float Ignore);
    float getPackVoltage();
    float getAvgTemperature();
    float getHighTemperature();
    float getLowTemperature();
    float getAvgCellVolt();
    float getLowCellVolt();
    float getHighCellVolt();
    float getHistLowestPackVolt();
    float getHistHighestPackVolt();
    float getHistLowestCellVolt();
    float getHistHighestCellVolt();
    float getHistHighestPackTemp();
    float getHistHighestCellDiffVolt();
    bool getIsFaulted();
    bool getLineFault();
    /*
      void processCANMsg(CAN_FRAME &frame);
    */
    void printAllCSV();
    void printPackSummary();
    void printPackDetails();


  private:
    float packVolt;                         // All modules added together
    int pstring;
    float lowCellVolt;
    float highCellVolt;
    float histLowestPackVolt;
    float histHighestPackVolt;
    float histLowestCellVolt;
    float histHighestCellVolt;
    float histHighestCellDiffVolt;
    float histLowestPackTemp;
    float histHighestPackTemp;
    float highTemp;
    float lowTemp;
    BMSModule modules[MAX_MODULE_ADDR + 1]; // store data for as many modules as we've configured for.
    int batteryID;
    int numFoundModules;                    // The number of modules that seem to exist
    //bool isFaulted;
    bool lineFault;     //true if we lose comms with modules.
    //int spack;
    /*
      void sendBatterySummary();
      void sendModuleSummary(int module);
      void sendCellDetails(int module, int cell);
    */

};

#endif //ifndef BMSMODULEMANAGER_HPP_
