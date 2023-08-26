#ifndef BMSMODULEMANAGER_HPP_
#define BMSMODULEMANAGER_HPP_
#include <Arduino.h>
#include "BMSModule.hpp"
#include "BMSDriver.hpp"

class BMSModuleManager
{
  public:
    BMSModuleManager(Settings* sett);
    int seriescells();
    void resetModuleRecordedValues();
    void StopBalancing();
    void balanceCells(uint8_t duration, float cell_v_offset);
    void renumberBoardIDs();
    void clearFaults();
    void sleepBoards();
    void wakeBoards();
    uint16_t getAllVoltTemp();
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
    time_t getHistLowestPackVoltTimeStamp();
    float getHistHighestPackVolt();
    time_t getHistHighestPackVoltTimeStamp();
    float getHistLowestCellVolt();
    float getHistHighestCellVolt();
    float getHistLowestPackTemp();
    time_t getHistLowestPackTempTimeStamp();
    float getHistHighestPackTemp();
    time_t getHistHighestPackTempTimeStamp();
    float getHistHighestCellDiffVolt();
    bool getIsFaulted();
    bool getLineFault();
    /*
      void processCANMsg(CAN_FRAME &frame);
    */
    void printAllCSV();
    void printPackSummary();
    void printPackGraph();


  private:
    float packVolt;                         // All modules added together
    int pstring;
    float lowCellVolt;
    float highCellVolt;
    float histLowestPackVolt; time_t histLowestPackVoltTimeStamp;
    float histHighestPackVolt; time_t histHighestPackVoltTimeStamp;
    float histLowestCellVolt;
    float histHighestCellVolt;
    float histHighestCellDiffVolt;
    float histLowestPackTemp; time_t histLowestPackTempTimeStamp;
    float histHighestPackTemp; time_t histHighestPackTempTimeStamp;
    float highTemp;
    float lowTemp;
    BMSModule modules[MAX_MODULE_ADDR + 1]; // store data for as many modules as we've configured for.
    int batteryID;
    int numFoundModules;                    // The number of modules that seem to exist
    bool lineFault;     //true if we lose comms with modules.

    Settings* settings;
};

#endif //ifndef BMSMODULEMANAGER_HPP_
