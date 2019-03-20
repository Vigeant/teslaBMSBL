#include <Arduino.h>

#ifndef BMSMODULE_HPP_
#define BMSMODULE_HPP_

class BMSModule
{
  public:
    BMSModule();
    //void readStatus();
    void resetRecordedValues();
    //void stopBalance();
    bool balanceCells(uint8_t cellMask, uint8_t balanceTime);
    bool updateInstanceWithModuleValues();
    
    //int getscells();
    float getCellVoltage(int cellIndex);
    float getLowCellV();
    float getHighCellV();
    float getAverageV();
    float getLowTemp();
    float getHighTemp();
    float getHighestModuleVolt();
    float getLowestModuleVolt();
    float getHighestCellVolt(int cell);
    float getLowestCellVolt(int cell);
    float getHighestTemp();
    float getLowestTemp();
    float getAvgTemp();
    float getModuleVoltage();
    float getTemperature(int temp);
    uint8_t getFaults();
    uint8_t getAlerts();
    uint8_t getCOVCells();
    uint8_t getCUVCells();
    void setAddress(uint8_t newAddr);
    uint8_t getAddress();

    void settempsensor(int tempsensor);
    //void setIgnoreCell(float Ignore);
    
    
  private:
    void logError(int16_t err);
    float cellVolt[6];          // calculated as 16 bit value * 6.250 / 16383 = volts
    float lowestCellVolt[6];
    float highestCellVolt[6];
    float moduleVolt;          // summed cell voltages
    float retmoduleVolt;          // calculated as 16 bit value * 33.333 / 16383 = volts
    float temperatures[2];     // Don't know the proper scaling at this point
    float lowestTemperature;
    float highestTemperature;
    float lowestModuleVolt;
    float highestModuleVolt;
    //float IgnoreCell;
    //bool exists;
    int alerts;
    int faults;
    int COVFaults;
    int CUVFaults;
    uint8_t moduleAddress;     //1 to 0x3E
    //int scells;
};

#endif //ifndef BMSMODULE_HPP_
