/*
   Adapted from the work of Tom Debree
   https://github.com/tomdebree/TeslaBMSV2/blob/master/BMSUtil.h
*/
#include "BMSDriver.hpp"

//instantiate the drive
BMSDriver bmsdriver_inst;

/////////////////////////////////////////////////
/// \brief The BMSDriver provides a high level API to talk to the Tesla module boards.
///
/// The constructor will initialize the arduino Serial port to the appropriate speed of 612500.
/////////////////////////////////////////////////
BMSDriver::BMSDriver() {
  SERIALBMS.begin(612500);
}

/////////////////////////////////////////////////
/// \brief Helper function to help interpret and log error codes returned by the driver.
///
/// @param moduleAddress The module address.
/// @param error The error code returned by the driver function (read or write).
/// @param message An extra message to display defined by the user.
/////////////////////////////////////////////////
void BMSDriver::logError(const uint8_t moduleAddress, const int16_t error, const char* message) {
  switch (error) {
    case ILLEGAL_READ_LEN:
      LOG_ERROR("Module %d: ILLEGAL_READ_LEN | %s\n", moduleAddress, message);
      break;
    case READ_CRC_FAIL:
      LOG_ERROR("Module %d: READ_CRC_FAIL | %s\n", moduleAddress, message);
      break;
    case READ_RECV_MODADDR_MISMATCH:
      LOG_ERROR("Module %d: READ_RECV_MODADDR_MISMATCH | %s\n", moduleAddress, message);
      break;
    case READ_RECV_ADDR_MISMATCH:
      LOG_ERROR("Module %d: READ_RECV_ADDR_MISMATCH | %s\n", moduleAddress, message);
      break;
    case READ_RECV_LEN_MISMATCH:
      LOG_ERROR("Module %d: READ_RECV_LEN_MISMATCH | %s\n", moduleAddress, message);
      break;
    case WRITE_RECV_LEN_MISMATCH:
      LOG_ERROR("Module %d: WRITE_RECV_LEN_MISMATCH | %s\n", moduleAddress, message);
      break;
    case WRITE_CRC_FAIL:
      LOG_ERROR("Module %d: WRITE_CRC_FAIL | %s\n", moduleAddress, message);
      break;

    default:
      LOG_ERROR("Module %d: UNKNOWN_ERROR | %s\n", moduleAddress, message);
      break;
  }

}

/////////////////////////////////////////////////
/// \brief reads values from the string of bms modules.
///
/// @param moduleAddress The module address to read from.
/// @param readAddress The address to read from in the module.
/// @param readLen The number of bytes to read from the module starting from readAddress.
/// @param recvBuff The buffer where the data will be written to.
/////////////////////////////////////////////////
int16_t BMSDriver::read(const uint8_t moduleAddress, const uint8_t readAddress, const uint8_t readLen, uint8_t* recvBuff ) {
  uint8_t fixedModAddress = moduleAddress << 1;
  uint8_t byteIndex = 0;
  uint8_t maxLen = readLen + 4;//[modAddr][readAddr][readLen][data][CRC]
  uint8_t buff[MAX_PAYLOAD];

  //clean out recv buffer
  while (SERIALBMS.available()) SERIALBMS.read();

  //check if the read is larger than our recv buffer
  if (maxLen > MAX_PAYLOAD) return ILLEGAL_READ_LEN;

  //clean buffers
  memset(buff, 0,  sizeof(buff));
  memset(recvBuff, 0,  readLen);

  //sending read command on serial port
  //LOG_DEBUG("Reading module:%3d, addr:0x%02x, len:%d\n", moduleAddress, readAddress, readLen );
  SERIALBMS.write(fixedModAddress);
  SERIALBMS.write(readAddress);
  SERIALBMS.write(readLen);

  //receiving answer
  delay(2 * ((readLen / 8) + 1));
  for (byteIndex = 0; SERIALBMS.available() && (byteIndex < maxLen); byteIndex++) {
    buff[byteIndex] = SERIALBMS.read();
    //chThdSleepMilliseconds(5);
  }

  //empty out the serial read buffer
  if (maxLen > byteIndex) {
    //LOG_ERROR("READ_RECV_LEN_MISMATCH | Reading module:%3d, addr:0x%02x, len:%d, maxlen:%d != byteIndex:%d\n", moduleAddress, readAddress, readLen, maxLen, byteIndex );
    return READ_RECV_LEN_MISMATCH;
  }
  while (SERIALBMS.available()) SERIALBMS.read();

  //verify the CRC
  if (genCRC(buff, maxLen - 1) != buff[maxLen - 1]) {
    LOG_ERROR("READ_CRC_FAIL | Reading module:%3d, addr:0x%02x, len:%d\n", moduleAddress, readAddress, readLen );
  }

  //success! remove 4 bytes protocol wrapper around payload
  memcpy(recvBuff, &buff[3], readLen); //[modAddr][readAddr][readLen][data][CRC] -> [data]
  return byteIndex;
}

/////////////////////////////////////////////////
/// \brief writes a byte to a module in the string of bms modules.
///
/// @param moduleAddress The module address to write to. Can use a boradcast.
/// @param writeAddress The address to write to in the module.
/// @param sendByte The byte to write.
/////////////////////////////////////////////////
int16_t BMSDriver::write(const uint8_t moduleAddress, const uint8_t writeAddress, const uint8_t sendByte) {
  uint8_t fixedModAddress = moduleAddress << 1;
  uint8_t byteIndex = 0;
  uint8_t maxLen = 4;//[modAddr][readAddr][data][CRC]
  uint8_t sendBuff[4];
  uint8_t recvBuff[4];

  //clean out recv buffer
  while (SERIALBMS.available()) SERIALBMS.read();
  //clean buffer
  memset(sendBuff, 0,  sizeof(sendBuff));
  memset(recvBuff, 0,  sizeof(recvBuff));

  //sending read command on serial port
  //LOG_DEBUG("Writing module:%d, addr:0x%x, byte:%d\n", moduleAddress, writeAddress, sendByte );
  sendBuff[0] = fixedModAddress | 1;
  sendBuff[1] = writeAddress;
  sendBuff[2] = sendByte;
  sendBuff[3] = genCRC(sendBuff, 3);
  SERIALBMS.write(sendBuff, sizeof(sendBuff));

  //receiving answer
  delay(2);
  for (byteIndex = 0; SERIALBMS.available() && (byteIndex < maxLen); byteIndex++) {
    recvBuff[byteIndex] = SERIALBMS.read();
  }

  //empty out the serial read buffer
  if (maxLen < byteIndex) {
    while (SERIALBMS.available()) SERIALBMS.read();
  } else if (maxLen > byteIndex) {
    return WRITE_RECV_LEN_MISMATCH;
  }

  //verify the CRC
  if (sendBuff[maxLen - 1] != recvBuff[maxLen - 1]) {
    LOG_ERROR("WRITE_CRC_FAIL | Writing module:%3d, addr:0x%02x, byte:%x\n", moduleAddress, writeAddress, sendByte );
  }
  return byteIndex;
}


uint8_t BMSDriver::genCRC(const uint8_t * buf, const uint8_t bufLen) {
  uint8_t generator = 0x07;
  uint8_t crc = 0;

  for (int x = 0; x < bufLen; x++)
  {
    crc ^= buf[x]; /* XOR-in the next input byte */
    for (int i = 0; i < 8; i++)
    {
      if ((crc & 0x80) != 0)
      {
        crc = (uint8_t)((crc << 1) ^ generator);
      }
      else
      {
        crc <<= 1;
      }
    }
  }
  return crc;
}
