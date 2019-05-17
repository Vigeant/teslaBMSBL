#include "Oled.hpp"
#include "Logger.hpp"

static TeensyView oled(OLED_PIN_RESET, OLED_PIN_DC, OLED_PIN_CS, OLED_PIN_SCK, OLED_PIN_MOSI);

static uint8_t sidewinder[] = {0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x80,  0xc0,  0xe0,  0xe0,  0xf0,  0xf0,  0xf8,  0xf8,  0x78,  0x7c,  0x3c,  0x3c,  0x3c,  0x3c,  0x3c,  0x3c,  0x3c,  0x3c,  0x3c,  0xf8,  0xf8,  0x60,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,
                               0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0xf8,  0xfe,  0xdf,  0x8f,  0x07,  0x03,  0x01,  0x01,  0x00,  0x00,  0x00,  0x00,  0x10,  0x10,  0x18,  0x18,  0x18,  0x1c,  0x9c,  0x0e,  0x0e,  0x07,  0x03,  0x01,  0x00,  0x00,  0x00,  0x00,  0x00,  0x80,  0x80,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x40,  0xe0,  0xc0,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x80,  0x80,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,
                               0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0xc0,  0xe0,  0xf0,  0xb8,  0xd8,  0xfc,  0x7c,  0x3c,  0x18,  0xc0,  0xe0,  0x20,  0x20,  0x20,  0x20,  0x63,  0x67,  0x47,  0x0f,  0x0f,  0x1f,  0x1e,  0x3e,  0xfc,  0xf8,  0x70,  0x00,  0xc0,  0xc0,  0xe0,  0x70,  0x37,  0x07,  0x87,  0xe0,  0xf0,  0x30,  0x18,  0x18,  0xd8,  0xf0,  0x78,  0x1e,  0x0f,  0xe3,  0xf1,  0xd8,  0x4c,  0x6c,  0xbc,  0xc8,  0xe0,  0x78,  0x3c,  0x9c,  0xc0,  0xf0,  0x38,  0x88,  0xc0,  0x7c,  0x3e,  0xee,  0xf0,  0x38,  0x9c,  0xcc,  0xe0,  0xf0,  0x3c,  0x9c,  0xec,  0x7c,  0x3c,  0x9c,  0xc0,  0xf0,  0x38,  0x18,  0x8c,  0xcc,  0xf8,  0x3c,  0x1e,  0xc7,  0xe3,  0xb1,  0x90,  0x58,  0x30,  0x00,  0xd0,  0xf0,  0x70,  0x20,  0xe0,  0xe0,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,
                               0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x0f,  0x1f,  0x1f,  0x3f,  0x18,  0x18,  0x1c,  0x0e,  0x06,  0x03,  0x0f,  0x0f,  0x1e,  0x1e,  0x1c,  0x1c,  0x1c,  0x1c,  0x1c,  0x1c,  0x0e,  0x0e,  0x0e,  0x07,  0x03,  0x01,  0x00,  0x00,  0x07,  0x07,  0x06,  0x06,  0x02,  0x01,  0x03,  0x03,  0x06,  0x02,  0x02,  0x03,  0x07,  0x07,  0x02,  0x02,  0x01,  0x03,  0x03,  0x02,  0x00,  0x01,  0x00,  0x01,  0x01,  0x01,  0x01,  0x01,  0x03,  0x02,  0x03,  0x01,  0x00,  0x00,  0x01,  0x01,  0x01,  0x01,  0x00,  0x01,  0x01,  0x00,  0x00,  0x01,  0x01,  0x01,  0x01,  0x00,  0x03,  0x03,  0x03,  0x01,  0x01,  0x03,  0x03,  0x01,  0x01,  0x03,  0x07,  0x04,  0x02,  0x03,  0x06,  0x07,  0x03,  0x00,  0x00,  0x01,  0x01,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00
                              };
static uint8_t teslalogo[] = {0x00,  0x00,  0x00,  0x00,  0x10,  0x10,  0x50,  0xd8,  0xd8,  0xc8,  0xc8,  0x68,  0x68,  0x68,  0xe8,  0xe8,  0xec,  0xec,  0x0c,  0x0c,  0xec,  0xec,  0xe8,  0xe8,  0x68,  0x68,  0x68,  0xc8,  0xc8,  0xd8,  0xd8,  0x50,  0x10,  0x10,  0x00,  0x00,  0x00,  0x04,  0x0c,  0x0c,  0x0c,  0x0c,  0x0c,  0xfc,  0xfc,  0x0c,  0x0c,  0x0c,  0x0c,  0x0c,  0x04,  0x00,  0x00,  0x00,  0x00,  0x00,  0x44,  0xcc,  0xcc,  0xcc,  0xcc,  0xcc,  0xcc,  0xcc,  0xcc,  0xcc,  0xcc,  0xcc,  0x44,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0xfc,  0xfc,  0xcc,  0xcc,  0xcc,  0xcc,  0xcc,  0xcc,  0xcc,  0xcc,  0xcc,  0xc4,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0xfc,  0xfc,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0xc4,  0xcc,  0xcc,  0xcc,  0xcc,  0xcc,  0xcc,  0xcc,  0xcc,  0xcc,  0xcc,  0xc4,  0x00,  0x00,  0x00,  0x00,  0x00,
                              0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x01,  0x01,  0x00,  0x00,  0x00,  0x00,  0x00,  0x1f,  0xff,  0xff,  0xff,  0xff,  0xff,  0xff,  0x1f,  0x00,  0x00,  0x00,  0x00,  0x00,  0x01,  0x01,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x0f,  0x0f,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x04,  0x0c,  0x0c,  0x0c,  0x0c,  0x0c,  0x0c,  0x0c,  0x0c,  0x0c,  0x0c,  0x0c,  0x04,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x08,  0x0c,  0x0c,  0x0c,  0x0c,  0x0c,  0x0c,  0x0c,  0x0c,  0x0c,  0x0f,  0x0f,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x0f,  0x0f,  0x0c,  0x0c,  0x0c,  0x0c,  0x0c,  0x0c,  0x0c,  0x0c,  0x0c,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x0f,  0x0f,  0x01,  0x01,  0x01,  0x01,  0x01,  0x01,  0x01,  0x01,  0x0f,  0x0f,  0x00,  0x00,  0x00,  0x00,  0x00,
                              0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x03,  0x7f,  0xff,  0xff,  0x7f,  0x03,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0xf8,  0xf8,  0x48,  0x48,  0xf8,  0xb0,  0x00,  0xf8,  0xf8,  0x70,  0xc0,  0x00,  0xc0,  0x70,  0xf8,  0xf8,  0x00,  0x70,  0xf8,  0xc8,  0xc8,  0x90,  0x00,  0x00,  0x00,  0x00,  0x00,  0xf8,  0xf8,  0x48,  0xc8,  0xf8,  0x30,  0x08,  0x08,  0xf8,  0xf8,  0x08,  0x08,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,
                              0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x0f,  0x0f,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x07,  0x07,  0x04,  0x04,  0x07,  0x03,  0x00,  0x07,  0x07,  0x00,  0x01,  0x07,  0x01,  0x00,  0x07,  0x07,  0x00,  0x02,  0x04,  0x04,  0x07,  0x03,  0x00,  0x00,  0x00,  0x00,  0x00,  0x07,  0x07,  0x00,  0x00,  0x07,  0x07,  0x00,  0x00,  0x07,  0x07,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00
                             };

/////////////////////////////////////////////////
/// \brief Constructor for the Oled teensyView display
/////////////////////////////////////////////////
Oled::Oled(Controller* cont_inst_ptr) {
  oled.begin();    // Initialize the OLED
  oled.clear(ALL); // Clear the display's internal memory
  oled.display();  // Display what's in the buffer (splashscreen)
  oled.clear(PAGE); // Clear the buffer.
  state = FMT6;
  controller_inst_ptr = cont_inst_ptr;
}

/*
  Vbat
  Tbat
*/
void Oled::printFormat1() {
  const int col0 = 0;
  const int col1 = oled.getLCDWidth() / 2;

  oled.clear(PAGE);            // Clear the display
  oled.setCursor(col0, 0);     // Set cursor to top-left
  oled.setFontType(1);         // Smallest font
  oled.print("Vbat");
  oled.setCursor(col1, 0);
  oled.print("Tbat");

  oled.setFontType(2);         // 7-segment font
  oled.setCursor(col0, oled.getLCDHeight() / 2);
  oled.print(controller_inst_ptr->getBMSPtr()->getPackVoltage());
  oled.setCursor(col1, oled.getLCDHeight() / 2);
  oled.print(controller_inst_ptr->getBMSPtr()->getAvgTemperature());
  oled.display();
}

/*
  VClo
  VChi      ou VCdiff au lieu de ces deux valeurs…
*/
void Oled::printFormat2() {
  const int col0 = 0;
  const int col1 = oled.getLCDWidth() / 2;

  oled.clear(PAGE);            // Clear the display
  oled.setCursor(col0, 0);        // Set cursor to top-left
  oled.setFontType(1);         // Smallest font
  oled.print("VClo");          // Print "A0"
  oled.setCursor(col1, 0);
  oled.print("VChi");

  oled.setFontType(2);         // 7-segment font
  oled.setCursor(col0, oled.getLCDHeight() / 2);
  oled.print(controller_inst_ptr->getBMSPtr()->getLowCellVolt());  // Print a0 reading
  oled.setCursor(col1, oled.getLCDHeight() / 2);
  oled.print(controller_inst_ptr->getBMSPtr()->getHighCellVolt());
  oled.display();
}

/*

  VCmin
  VCmax
*/
void Oled::printFormat3() {
  const int col0 = 0;
  const int col1 = oled.getLCDWidth() / 2;

  oled.clear(PAGE);            // Clear the display
  oled.setCursor(col0, 0);        // Set cursor to top-left
  oled.setFontType(1);         // Smallest font
  oled.print("VCmin");
  oled.setCursor(col1, 0);
  oled.print("VCmax");

  oled.setFontType(2);         // 7-segment font
  oled.setCursor(col0, oled.getLCDHeight() / 2);
  oled.print(controller_inst_ptr->getBMSPtr()->getHistLowestCellVolt());
  oled.setCursor(col1, oled.getLCDHeight() / 2);
  oled.print(controller_inst_ptr->getBMSPtr()->getHistHighestCellVolt());
  oled.display();
}

/*
  VCdmax
  Tmax
*/
void Oled::printFormat4() {
  const int col0 = 0;
  const int col1 = oled.getLCDWidth() / 2;

  oled.clear(PAGE);            // Clear the display
  oled.setCursor(col0, 0);        // Set cursor to top-left
  oled.setFontType(1);         // Smallest font
  //oled.print("VCdmax");
  oled.print("VCdiff");
  oled.setCursor(col1, 0);
  oled.print("Tmax");

  oled.setFontType(2);         // 7-segment font
  oled.setCursor(col0, oled.getLCDHeight() / 2);
  //oled.print(controller_inst_ptr->getBMSPtr()->getHistHighestCellDiffVolt());
  oled.print(controller_inst_ptr->getBMSPtr()->getHighCellVolt() - controller_inst_ptr->getBMSPtr()->getLowCellVolt());
 
  oled.setCursor(col1, oled.getLCDHeight() / 2);
  //oled.setFontType(1);
  oled.print(controller_inst_ptr->getBMSPtr()->getHistHighestPackTemp());
  oled.display();
}

void Oled::printFormat5() {
  switch (controller_inst_ptr->getState()) {
    case Controller::INIT:
      Oled::printCentre("INIT", 1);
      break;
    case Controller::STANDBY:
      Oled::printCentre("STANDBY", 1);
      break;
    case Controller::STANDBY_DC2DC:
      Oled::printCentre("STANDBY_DC2", 1);
      break;
    case Controller::CHARGER_CYCLE:
      Oled::printCentre("CHARGER_CYC", 1);
      break;
    case Controller::PRE_CHARGE:
      Oled::printCentre("PRE_CHARGE", 1);
      break;
    case Controller::CHARGING:
      Oled::printCentre("CHARGING", 1);
      break;
    case Controller::RUN:
      Oled::printCentre("RUN", 1);
      break;
    case Controller::EVSE_CONNECTED_DC2DC:
      Oled::printCentre("EVSE_CON_DC2DC", 1);
      break;
    case Controller::EVSE_CONNECTED:
      Oled::printCentre("EVSE_CONNECTED", 1);
      break;
    default:
      Oled::printCentre("default", 1);
      break;
  }
}

void Oled::printTeslaBMSRT() {
  oled.drawBitmap(teslalogo);
  oled.display();
}

void Oled::printESidewinder() {
  oled.drawBitmap(sidewinder);
  oled.display();
}

void Oled::printStickyFaults() {
  const int col0 = 0;

  oled.clear(PAGE);            // Clear the display
  oled.setCursor(col0, 0);        // Set cursor to top-left
  oled.setFontType(1);         // Smallest font
  oled.print("sFault Codes");
  oled.setCursor(col0, oled.getLCDHeight() / 2);

  if (controller_inst_ptr->sFaultModuleLoop) {
    oled.print("A");
  }
  if (controller_inst_ptr->sFaultBatMon) {
    oled.print("B");
  }
  if (controller_inst_ptr->sFaultBMSSerialComms) {
    oled.print("C");
  }
  if (controller_inst_ptr->sFaultBMSOV) {
    oled.print("D");
  }
  if (controller_inst_ptr->sFaultBMSUV) {
    oled.print("E");
  }
  if (controller_inst_ptr->sFaultBMSOT) {
    oled.print("F");
  }
  if (controller_inst_ptr->sFaultBMSUT) {
    oled.print("G");
  }
  if (controller_inst_ptr->sFault12VBatOV) {
    oled.print("H");
  }
  if (controller_inst_ptr->sFault12VBatUV) {
    oled.print("I");
  }

  oled.display();
}

void Oled::printFaults() {
  const int col0 = 0;

  oled.clear(PAGE);            // Clear the display
  oled.setCursor(col0, 0);        // Set cursor to top-left
  oled.setFontType(1);         // Smallest font
  oled.print("Fault Codes");
  oled.setCursor(col0, oled.getLCDHeight() / 2);

  if (controller_inst_ptr->faultModuleLoop) {
    oled.print("A");
  }
  if (controller_inst_ptr->faultBatMon) {
    oled.print("B");
  }
  if (controller_inst_ptr->faultBMSSerialComms) {
    oled.print("C");
  }
  if (controller_inst_ptr->faultBMSOV) {
    oled.print("D");
  }
  if (controller_inst_ptr->faultBMSUV) {
    oled.print("E");
  }
  if (controller_inst_ptr->faultBMSOT) {
    oled.print("F");
  }
  if (controller_inst_ptr->faultBMSUT) {
    oled.print("G");
  }
  if (controller_inst_ptr->fault12VBatOV) {
    oled.print("H");
  }
  if (controller_inst_ptr->fault12VBatUV) {
    oled.print("I");
  }

  oled.display();
}

/////////////////////////////////////////////////
/// \brief doOled is the function that executes a tick of the Oled state machine.
///
/// The Oled cycles through formats after a predefined number of ticks. At each tick, it updates what is currently displayed in the current format.
/////////////////////////////////////////////////
void Oled::doOled() {
  const int stateticks = 6;
  static int ticks = 0;
  switch (state) {
    case FMT1:
      Oled::printFormat1();
      if (ticks >= stateticks) {
        ticks = 0;
        state = FMT2;
      }
      break;
    case FMT2:
      Oled::printFormat2();
      if (ticks >= stateticks) {
        ticks = 0;
        state = FMT3;
      }
      break;
    case FMT3:
      Oled::printFormat3();
      if (ticks >= stateticks) {
        ticks = 0;
        state = FMT4;
      }
      break;
    case FMT4:
      Oled::printFormat4();
      if (ticks >= stateticks) {
        ticks = 0;
        state = FMT5;
      }
      break;
    case FMT5:
      Oled::printFormat5();
      if (ticks >= stateticks) {
        ticks = 0;
        state = FMT6;
      }
      break;
    case FMT6:
      Oled::printTeslaBMSRT();
      if (ticks >= stateticks) {
        ticks = 0;
        state = FMT7;
      }
      break;
    case FMT7:
      Oled::printESidewinder();
      if (ticks >= stateticks) {
        ticks = 0;
        if (controller_inst_ptr->isFaulted) {
          state = FMT8;
        } else if (controller_inst_ptr->stickyFaulted) {
          state = FMT9;
        } else {
          state = FMT1;
        }
      }
      break;
    case FMT8:
      Oled::printFaults();
      if (ticks >= stateticks) {
        ticks = 0;
        state = FMT9;
      }
      break;
    case FMT9:
      Oled::printStickyFaults();
      if (ticks >= stateticks) {
        ticks = 0;
        state = FMT1;
      }
      break;
    default:
      break;
  }
  ticks++;

}

// Center and print a small values string
// This function is quick and dirty. Only works for titles one
// line long.
void Oled::printCentre(const char* value, int font)
{
  int middleX = oled.getLCDWidth() / 2;
  int middleY = oled.getLCDHeight() / 2;

  oled.clear(PAGE);
  oled.setFontType(font);
  // Try to set the cursor in the middle of the screen
  oled.setCursor(middleX - (oled.getFontWidth() * (strlen(value) / 2)) - 4,
                 middleY - (oled.getFontHeight() / 2) + 3);
  // Print the title:
  oled.print(value);
  oled.display();
}
