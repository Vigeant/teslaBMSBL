/**@file BMSDriver.hpp */
#include <Arduino.h>
#include "Logger.hpp"
#include <string.h>

//Define this to be the serial port the Tesla BMS modules are connected to.
//On the Due you need to use a USART port (Serial1, Serial2, Serial3) and update the call to serialSpecialInit if not Serial1
//Serial3 for teensy
#define SERIALBMS  Serial3

#define REG_DEV_STATUS      0
#define REG_GPAI            1
#define REG_VCELL1          3
#define REG_VCELL2          5
#define REG_VCELL3          7
#define REG_VCELL4          9
#define REG_VCELL5          0xB
#define REG_VCELL6          0xD
#define REG_TEMPERATURE1    0xF
#define REG_TEMPERATURE2    0x11
#define REG_ALERT_STATUS    0x20
#define REG_FAULT_STATUS    0x21
#define REG_COV_FAULT       0x22
#define REG_CUV_FAULT       0x23
#define REG_ADC_CTRL        0x30
#define REG_IO_CTRL         0x31
#define REG_BAL_CTRL        0x32
#define REG_BAL_TIME        0x33
#define REG_ADC_CONV        0x34
#define REG_ADDR_CTRL       0x3B
#define REG_SETPNTS_CTRL    0x40

#define MAX_MODULE_ADDR     0x3E

#define BROADCAST_ADDR      0x3F

#define MAX_PAYLOAD 128

//error codes
#define ILLEGAL_READ_LEN -2
#define READ_CRC_FAIL -3
#define READ_RECV_MODADDR_MISMATCH -4
#define READ_RECV_ADDR_MISMATCH -5
#define READ_RECV_LEN_MISMATCH -6
#define WRITE_RECV_LEN_MISMATCH -7
#define WRITE_CRC_FAIL -8

class BMSDriver {
  public:
    BMSDriver();
    int16_t read(const uint8_t moduleAddress, const uint8_t readAddress, const uint8_t readLen, uint8_t* recvBuff);
    int16_t write(const uint8_t moduleAddress, const uint8_t writeAddress, const uint8_t sendByte);
    void logError(const uint8_t ma, const int16_t err, const char* message);

  private:
    uint8_t genCRC(const uint8_t * buf, const uint8_t bufLen);
  
};

//export the logger
extern BMSDriver bmsdriver_inst;

/////////////////////////////////////////////////
/// \brief Helper macro that reads values from the string of bms modules.
///
/// @param moduleAddress The module address to read from.
/// @param readAddress The address to read from in the module.
/// @param readLen The number of bytes to read from the module starting from readAddress.
/// @param recvBuff The buffer where the data will be written to.
/////////////////////////////////////////////////
#define BMSDR bmsdriver_inst.read

/////////////////////////////////////////////////
/// \brief Helper macro that writes a byte to a module in the string of bms modules.
///
/// @param moduleAddress The module address to write to. Can use a boradcast.
/// @param writeAddress The address to write to in the module.
/// @param sendByte The byte to write.
/////////////////////////////////////////////////
#define BMSDW bmsdriver_inst.write

/////////////////////////////////////////////////
/// \brief Helper macro that will print the location of the error and function to help interpret and log error codes returned by the driver.
///
/// @param moduleAddress The module address.
/// @param error The error code returned by the driver function (read or write).
/// @param message An extra message to display defined by the user.
/////////////////////////////////////////////////
#define BMSD_LOG_ERR LOG_ERR("file: %s, function: %s, line: %d\n",strrchr(__FILE__,'\\'),__func__,__LINE__); bmsdriver_inst.logError
