#include "Controller.hpp"

/////////////////////////////////////////////////
/// \brief Orchestrates the activities within the BMS via a state machine.
/////////////////////////////////////////////////
void Controller::doController() {
  static int ticks = 0;
  static unsigned int bat12Vcyclestart = 0;
  static int standbyTicks = 1;
  const int stateticks = 4;
  bat12vVoltage = (float)analogRead(INA_12V_BAT) / BAT12V_SCALING_DIVISOR ;

  if (state != INIT) syncModuleDataObjects();

  //figure out state transition
  switch (state) {
    /**************** ****************/
    case INIT:
      if (ticks >= stateticks) {
        ticks = 0;
        state = STANDBY;
      }
      break;
    /**************** ****************/
    case STANDBY:
#ifdef STATECYCLING
      if (ticks >= stateticks) {
        ticks = 0;
        state = PRE_CHARGE;
      }
#else
      if (dc2dcON == 0 && bat12vVoltage < DC2DC_CYCLE_V_SETPOINT) {
        bat12Vcyclestart = (millis() / 1000);
        dc2dcON = 1;
      } else if ( dc2dcON == 1 && (bat12Vcyclestart + DC2DC_CYCLE_TIME_S) < (millis() / 1000)) {
        // I am ignoring the time counter loop around as it will simply result in a short charge cycle
        // followed with a proper charging cycle.
        dc2dcON = 0;
      }

      if (digitalRead(INH_RUN) == HIGH) {
        ticks = 0;
        state = RUN;
      } else if (bms.getLowCellVolt() < CHARGER_CYCLE_V_SETPOINT && bms.getHighCellVolt() < MAX_CHARGE_V_SETPOINT && ticks >= standbyTicks) {
        ticks = 0;
        state = PRE_CHARGE;
      }
#endif
      break;
    /**************** ****************/
    case PRE_CHARGE:
#ifdef STATECYCLING
      if (ticks >= stateticks) {
        ticks = 0;
        state = CHARGING;
      }
#else
      if (ticks >= 4) { //adjust to give time to the EVCC to properly boot
        if (digitalRead(INL_EVSE_DISC) == LOW) {
          ticks = 0;
          state = STANDBY;
        } else if (digitalRead(INL_CHARGING) == LOW) {
          ticks = 0;
          state = CHARGING;
        }
      }
#endif
      break;
    /**************** ****************/
    case CHARGING:
#ifdef STATECYCLING
      if (ticks >= stateticks) {
        ticks = 0;
        state = RUN;
      }
#else
      if (digitalRead(INL_EVSE_DISC) == LOW || digitalRead(INL_CHARGING) == HIGH) {
        ticks = 0;
        state = STANDBY;
      }
#endif
      break;
    /**************** ****************/
    case RUN:
#ifdef STATECYCLING
      if (ticks >= stateticks) {
        ticks = 0;
        state = INIT;
      }
#else
      if (digitalRead(INH_RUN ) == LOW) {
        ticks = 0;
        state = STANDBY;
      }
#endif
      break;
    /**************** ****************/
    default:
      break;
  }

  //execute state
  switch (state) {

    case INIT:
      period = 200;
      init();
      break;

    case STANDBY:
      //prevents sleeping if the console is connected or if within 1 minute of a hard reset.
      //The teensy wont let reprogram if it slept once so this allows reprograming within 1 minute.
      if (SERIALCONSOLE || millis() < 60000){
        period = 200;
        standbyTicks = 12;
      } else {
        period = 2500;
        standbyTicks = 1;
      }
      standby();
      break;

    case PRE_CHARGE:
      period = 200;
      pre_charge();
      break;

    case CHARGING:
      period = 200;
      charging();
      break;

    case RUN:
      period = 200;
      run();
      break;

    default:
      period = 200;
      break;
  }
  ticks++;
}

/////////////////////////////////////////////////
/// \brief When instantiated, the controller is in the init state ensuring that all the signal pins are set properly.
/////////////////////////////////////////////////
Controller::Controller() {
  state = INIT;
}

/////////////////////////////////////////////////
/// \brief gather all the data from the boards and check for any faults.
/////////////////////////////////////////////////
void Controller::syncModuleDataObjects() {
  float bat12vVoltage;
  bms.wakeBoards();
  bms.getAllVoltTemp();

  if (bms.getLineFault()) {
    faultBMSSerialCommsDB += 1;
    if (faultBMSSerialCommsDB >= FAULT_DEBOUNCE_COUNT) {
      if (!faultBMSSerialComms) {
        LOG_ERROR("Serial communication with battery modules lost!\n");
      }
      faultBMSSerialComms = true;
      faultBMSSerialCommsDB = FAULT_DEBOUNCE_COUNT;
    }
  } else {
    if (faultBMSSerialComms) LOG_INFO("Serial communication with battery modules re-established!\n");
    faultBMSSerialCommsDB = 0;
    faultBMSSerialComms = false;
  }

  if (digitalRead(INL_BAT_PACK_FAULT) == LOW) {
    faultModuleLoopDB += 1;
    if (faultModuleLoopDB >= FAULT_DEBOUNCE_COUNT) {
      if (!faultModuleLoop) {
        LOG_ERROR("One or more BMS modules have asserted the fault loop!\n");
      }
      faultModuleLoop = true;
      faultModuleLoopDB = FAULT_DEBOUNCE_COUNT;
    }
  } else {
    if (faultModuleLoop) LOG_INFO("All modules have deasserted the fault loop\n");
    faultModuleLoopDB = 0;
    faultModuleLoop = false;
  }

  if (digitalRead(INL_BAT_MON_FAULT) == LOW) {
    faultBatMonDB += 1;
    if (faultBatMonDB >= FAULT_DEBOUNCE_COUNT) {
      if (!faultBatMon) {
        LOG_ERROR("The battery monitor asserted the fault loop!\n");
      }
      faultBatMon = true;
      faultBatMonDB = FAULT_DEBOUNCE_COUNT;
    }
  } else {
    if (faultBatMon) LOG_INFO("The battery monitor deasserted the fault loop\n");
    faultBatMonDB = 0;
    faultBatMon = false;
  }

  if (digitalRead(INL_WATER_SENS1) == LOW) {
    faultWatSen1DB += 1;
    if (faultWatSen1DB >= FAULT_DEBOUNCE_COUNT) {
      if (!faultWatSen1) {
        LOG_ERROR("The battery water sensor 1 is reporting water!\n");
      }
      faultWatSen1 = true;
      faultWatSen1DB = FAULT_DEBOUNCE_COUNT;
    }
  } else {
    if (faultWatSen1) LOG_INFO("The battery water sensor 1 is reporting dry.\n");
    faultWatSen1DB = 0;
    faultWatSen1 = false;
  }

  if (digitalRead(INL_WATER_SENS2) == LOW) {
    faultWatSen2DB += 1;
    if (faultWatSen2DB >= FAULT_DEBOUNCE_COUNT) {
      if (!faultWatSen2) {
        LOG_ERROR("The battery water sensor 2 is reporting water!\n");
      }
      faultWatSen2 = true;
      faultWatSen2DB = FAULT_DEBOUNCE_COUNT;
    }
  } else {
    if (faultWatSen2) LOG_INFO("The battery water sensor 2 is reporting dry.\n");
    faultWatSen2DB = 0;
    faultWatSen2 = false;
  }

  if ( bms.getHighCellVolt() > OVER_V_SETPOINT) {
    faultBMSOVDB += 1;
    if (faultBMSOVDB >= FAULT_DEBOUNCE_COUNT) {
      if (!faultBMSOV) {
        LOG_ERROR("OVER_V_SETPOINT: %.2fV, highest cell:%.2fV\n", OVER_V_SETPOINT, bms.getHighCellVolt());
      }
      faultBMSOV = true;
      faultBMSOVDB = FAULT_DEBOUNCE_COUNT;
    }
  } else {
    if (faultBMSOV) LOG_INFO("All cells are back under OV threshold\n");
    faultBMSOVDB = 0;
    faultBMSOV = false;
  }

  if ( bms.getLowCellVolt() < UNDER_V_SETPOINT) {
    faultBMSUVDB += 1;
    if (faultBMSUVDB >= FAULT_DEBOUNCE_COUNT) {
      if (!faultBMSUV) {
        LOG_ERROR("UNDER_V_SETPOINT: %.2fV, lowest cell:%.2fV\n", UNDER_V_SETPOINT, bms.getLowCellVolt());
      }
      faultBMSUV = true;
      faultBMSUVDB = FAULT_DEBOUNCE_COUNT;
    }
  } else {
    if (faultBMSUV) LOG_INFO("All cells are back over UV threshold\n");
    faultBMSUVDB = 0;
    faultBMSUV = false;
  }

  if ( bms.getHighTemperature() > OVER_T_SETPOINT) {
    faultBMSOTDB += 1;
    if (faultBMSOTDB >= FAULT_DEBOUNCE_COUNT) {
      if (!faultBMSOT) {
        LOG_ERROR("OVER_T_SETPOINT: %.2fV, highest module:%.2fV\n", UNDER_V_SETPOINT, bms.getHighTemperature());
      }
      faultBMSOT = true;
      faultBMSOTDB = FAULT_DEBOUNCE_COUNT;
    }
  } else {
    if (faultBMSOT) LOG_INFO("All modules are back under the OT threshold\n");
    faultBMSOTDB = 0;
    faultBMSOT = false;
  }

  if ( bms.getLowTemperature() < UNDER_T_SETPOINT) {
    faultBMSUTDB += 1;
    if (faultBMSUTDB >= FAULT_DEBOUNCE_COUNT) {
      if (!faultBMSUT) {
        LOG_ERROR("UNDER_T_SETPOINT: %.2fV, lowest module:%.2fV\n", UNDER_T_SETPOINT, bms.getLowTemperature());
      }
      faultBMSUT = true;
      faultBMSUTDB = FAULT_DEBOUNCE_COUNT;
    }
  } else {
    if (faultBMSUT) LOG_INFO("All modules are back over the UT threshold\n");
    faultBMSUTDB = 0;
    faultBMSUT = false;
  }

  bat12vVoltage = (float)analogRead(INA_12V_BAT) / BAT12V_SCALING_DIVISOR ;
  //LOG_INFO("bat12vVoltage: %.2f\n", bat12vVoltage);
  if ( bat12vVoltage > BAT12V_OVER_V_SETPOINT) {
    fault12VBatOVDB += 1;
    if (fault12VBatOVDB >= FAULT_DEBOUNCE_COUNT) {
      if (!fault12VBatOV) {
        LOG_ERROR("12VBAT_OVER_V_SETPOINT: %.2fV, V:%.2fV\n", BAT12V_OVER_V_SETPOINT, bat12vVoltage);
      }
      fault12VBatOV = true;
      fault12VBatOVDB = FAULT_DEBOUNCE_COUNT;
    }
  } else {
    if (fault12VBatOV) LOG_INFO("12V battery back under the OV threshold\n");
    fault12VBatOVDB = 0;
    fault12VBatOV = false;
  }

  if ( bat12vVoltage < BAT12V_UNDER_V_SETPOINT) {
    fault12VBatUVDB += 1;
    if (fault12VBatUVDB >= FAULT_DEBOUNCE_COUNT) {
      if (!fault12VBatUV) {
        LOG_ERROR("12VBAT_UNDER_V_SETPOINT: %.2fV, V:%.2fV\n", BAT12V_UNDER_V_SETPOINT, bat12vVoltage);
      }
      fault12VBatUV = true;
      fault12VBatUVDB = FAULT_DEBOUNCE_COUNT;
    }
  } else {
    if (fault12VBatUV) LOG_INFO("12V battery back over the UV threshold\n");
    fault12VBatUVDB = 0;
    fault12VBatUV = false;
  }

  chargerInhibit = faultModuleLoop || faultBatMon || faultBMSSerialComms || faultBMSOV || faultBMSOT || faultWatSen1 || faultWatSen2;
  powerLimiter = faultModuleLoop || faultBatMon || faultBMSSerialComms || faultBMSUV || faultBMSOT;
  isFaulted =  chargerInhibit || faultBMSUV || faultBMSUT || fault12VBatOV || fault12VBatUV;

  //update stiky faults
  sFaultModuleLoop |= faultModuleLoop;
  sFaultBatMon |= faultBatMon;
  sFaultBMSSerialComms |= faultBMSSerialComms;
  sFaultBMSOV |= faultBMSOV;
  sFaultBMSUV |= faultBMSUV;
  sFaultBMSOT |= faultBMSUV;
  sFaultBMSUT |= faultBMSUT;
  sFault12VBatOV |= fault12VBatOV;
  sFault12VBatUV |= fault12VBatUV;
  sFaultWatSen1 |= faultWatSen1;
  sFaultWatSen2 |= faultWatSen2;

  //update time stamps
  if (faultModuleLoop) faultModuleLoopTS = millis() / 1000;
  if (faultBatMon) faultBatMonTS = millis() / 1000;
  if (faultBMSSerialComms) faultBMSSerialCommsTS = millis() / 1000;
  if (faultBMSOV) faultBMSOVTS = millis() / 1000;
  if (faultBMSUV) faultBMSUVTS = millis() / 1000;
  if (faultBMSOT) faultBMSOTTS = millis() / 1000;
  if (faultBMSUT) faultBMSUTTS = millis() / 1000;
  if (fault12VBatOV) fault12VBatOVTS = millis() / 1000;
  if (fault12VBatUV) fault12VBatUVTS = millis() / 1000;
  if (faultWatSen1) faultWatSen1TS = millis() / 1000;
  if (faultWatSen2) faultWatSen2TS = millis() / 1000;

  stickyFaulted |= isFaulted;
  bms.clearFaults();
  //bms.sleepBoards();
}

/////////////////////////////////////////////////
/// \brief balances the cells according to BALANCE_CELL_V_OFFSET threshold in the CONFIG.h file
/////////////////////////////////////////////////
void Controller::balanceCells() {
  //balance for 1 second given that the controller wakes up every second.
  if (bms.getHighCellVolt() > PRECISION_BALANCE_V_SETPOINT) {
    //LOG_CONSOLE("precision balance\n");
    bms.balanceCells(5, PRECISION_BALANCE_CELL_V_OFFSET);
  } else if (bms.getHighCellVolt() > ROUGH_BALANCE_V_SETPOINT) {
    //LOG_CONSOLE("rough balance\n");
    bms.balanceCells(5, ROUGH_BALANCE_CELL_V_OFFSET);
  }
}

/////////////////////////////////////////////////
/// \brief computes the duty cycle required for the pwm controlling the coolant pump.
/// The duty cycle is inverted to allow amplifying the 0-3.3V to 0-5V using a transistor.
/// Returns a float from 0.0 - 1.0 that must be adjusted to the PWM range (0-255).
///
/// @param the temparature in C
/////////////////////////////////////////////////
//pwd = a*temp + b
#define COOLING_A (1.0 - FLOOR_DUTY_COOLANT_PUMP) / (COOLING_HIGHT_SETPOINT - COOLING_LOWT_SETPOINT)
#define COOLING_B FLOOR_DUTY_COOLANT_PUMP - COOLING_A * COOLING_LOWT_SETPOINT
float Controller::getCoolingPumpDuty(float temp) {
  if (temp < COOLING_LOWT_SETPOINT) {
    return FLOOR_DUTY_COOLANT_PUMP;
  } else if (temp > COOLING_HIGHT_SETPOINT) {
    return 1.0;
  } else {
    //return COOLING_A * temp + COOLING_B;
    return 1 - (COOLING_A * temp + COOLING_B); //flipped polarity to allow amplifying to 0-5V using a transistor
  }
}

/////////////////////////////////////////////////
/// \brief reset all boards, assign address to each board and configure their thresholds
/////////////////////////////////////////////////
void Controller::init() {
  pinMode(OUTL_12V_BAT_CHRG, OUTPUT);
  pinMode(OUTPWM_PUMP, OUTPUT); //PWM use analogWrite(OUTPWM_PUMP, 0-255);
  analogWrite(OUTPWM_PUMP, 255); //immediately set to off (inverted)
  pinMode(INL_BAT_PACK_FAULT, INPUT_PULLUP);
  pinMode(INL_BAT_MON_FAULT, INPUT_PULLUP);
  pinMode(INL_EVSE_DISC, INPUT_PULLUP);
  pinMode(INH_RUN, INPUT_PULLDOWN);
  pinMode(INL_CHARGING, INPUT_PULLUP);
  pinMode(INA_12V_BAT, INPUT);  // [0-1023] = analogRead(INA_12V_BAT)
  pinMode(OUTL_EVCC_ON, OUTPUT);
  pinMode(OUTL_NO_FAULT, OUTPUT);
  pinMode(INL_WATER_SENS1, INPUT_PULLUP);
  pinMode(INL_WATER_SENS2, INPUT_PULLUP);

  //faults
  faultModuleLoop = false;
  faultBatMon = false;
  faultBMSSerialComms = false;
  faultBMSOV = false;
  faultBMSUV = false;
  faultBMSOT = false;
  faultBMSUT = false;
  fault12VBatOV = false;
  fault12VBatUV = false;
  faultWatSen1 = false;
  faultWatSen2 = false;

  //sticky faults
  sFaultModuleLoop = false;
  sFaultBatMon = false;
  sFaultBMSSerialComms = false;
  sFaultBMSOV = false;
  sFaultBMSUV = false;
  sFaultBMSOT = false;
  sFaultBMSUT = false;
  sFault12VBatOV = false;
  sFault12VBatUV = false;
  sFaultWatSen1 = false;
  sFaultWatSen2 = false;

  //faults debounce counters
  faultModuleLoopDB = 0;
  faultBatMonDB = 0;
  faultBMSSerialCommsDB = 0;
  faultBMSOVDB = 0;
  faultBMSUVDB = 0;
  faultBMSOTDB = 0;
  faultBMSUTDB = 0;
  fault12VBatOVDB = 0;
  fault12VBatUVDB = 0;
  faultWatSen1DB = 0;
  faultWatSen2DB = 0;

  //faults time stamps (TS)
  faultModuleLoopTS = 0;
  faultBatMonTS = 0;
  faultBMSSerialCommsTS = 0;
  faultBMSOVTS = 0;
  faultBMSUVTS = 0;
  faultBMSOTTS = 0;
  faultBMSUTTS = 0;
  fault12VBatOVTS = 0;
  fault12VBatUVTS = 0;
  faultWatSen1TS = 0;
  faultWatSen2TS = 0;

  isFaulted = false;
  stickyFaulted = false;

  chargerInhibit = false;
  powerLimiter = false;
  dc2dcON = false;
  period = 200;

  bms.renumberBoardIDs();
  bms.clearFaults();
}

/////////////////////////////////////////////////
/// \brief standby state is when the boat *is not connected*
/// to a EVSE, not in run state and the 12V battery is above
/// its low voltage threshold.
/////////////////////////////////////////////////
void Controller::standby() {
  balanceCells();
  digitalWrite(OUTL_EVCC_ON, HIGH);
  digitalWrite(OUTL_NO_FAULT, chargerInhibit);
  digitalWrite(OUTL_12V_BAT_CHRG, !dc2dcON);
  analogWrite(OUTPWM_PUMP, 255); //max = off once amplified
}

/////////////////////////////////////////////////
/// \brief pre_charge state is turning on the EVCC to charge the battery. If the EVSE is disconnected,
/// it goes back to STANDBY.
/////////////////////////////////////////////////
void Controller::pre_charge() {
  balanceCells();
  digitalWrite(OUTL_EVCC_ON, LOW);
  digitalWrite(OUTL_NO_FAULT, chargerInhibit);
  digitalWrite(OUTL_12V_BAT_CHRG, !dc2dcON);
  analogWrite(OUTPWM_PUMP, 255); //max = off once amplified
}

/////////////////////////////////////////////////
/// \brief charging state is when the boat *is connected*
/// to a EVSE and is actively charging until EVCC shuts itself down.
/////////////////////////////////////////////////
void Controller::charging() {
  balanceCells();
  digitalWrite(OUTL_EVCC_ON, LOW);
  digitalWrite(OUTL_NO_FAULT, chargerInhibit);
  digitalWrite(OUTL_12V_BAT_CHRG, LOW);
  analogWrite(OUTPWM_PUMP, (uint8_t) (getCoolingPumpDuty(bms.getHighTemperature()) * 255 ));
}

/////////////////////////////////////////////////
/// \brief run state is turned on and ready to operate.
/////////////////////////////////////////////////
void Controller::run() {
  digitalWrite(OUTL_EVCC_ON, HIGH);
  digitalWrite(OUTL_NO_FAULT, powerLimiter);
  digitalWrite(OUTL_12V_BAT_CHRG, LOW);
  analogWrite(OUTPWM_PUMP, (uint8_t) (getCoolingPumpDuty(bms.getHighTemperature()) * 255 ));
}

/////////////////////////////////////////////////
/// \brief returns the current state the controller is in.
/////////////////////////////////////////////////
Controller::ControllerState Controller::getState() {
  return state;
}

/////////////////////////////////////////////////
/// \brief returns the BMS instance to allow access to its members for reporting purposes.
/////////////////////////////////////////////////
BMSModuleManager* Controller::getBMSPtr() {
  return &bms;
}

/////////////////////////////////////////////////
/// \brief returns the main loop period the controller is expecting.
/////////////////////////////////////////////////
uint32_t Controller::getPeriodMillis() {
  return period;
}

void Controller::printControllerState() {
  uint32_t seconds = millis() / 1000;
  LOG_CONSOLE("====================================================================================\n");
  LOG_CONSOLE("=                     BMS Controller registered faults                             =\n");
  switch (state) {
    case INIT:
      LOG_CONSOLE("=  state: INIT                                                                     =\n");
      break;
    case STANDBY:
      LOG_CONSOLE("=  state: STANDBY                                                                  =\n");
      break;
    case PRE_CHARGE:
      LOG_CONSOLE("=  state: PRE_CHARGE                                                               =\n");
      break;
    case CHARGING:
      LOG_CONSOLE("=  state: CHARGING                                                                 =\n");
      break;
    case RUN:
      LOG_CONSOLE("=  state: RUN                                                                      =\n");
      break;
  }
  LOG_CONSOLE("=  Time since last reset:%-3d days, %02d:%02d:%02d                                        =\n",
              seconds / 86400, (seconds % 86400) / 3600, (seconds % 3600) / 60, (seconds % 60));
  LOG_CONSOLE("====================================================================================\n");
  LOG_CONSOLE("%-22s   last fault time\n", "Fault Name");
  LOG_CONSOLE("----------------------   -----------------------------------------------------------\n");
  if (sFaultModuleLoop) LOG_CONSOLE("%-22s @ %-3d days, %02d:%02d:%02d\n",
                                      "faultModuleLoop", faultModuleLoopTS / 86400, (faultModuleLoopTS % 86400) / 3600, (faultModuleLoopTS % 3600) / 60, (faultModuleLoopTS % 60));
  if (sFaultBatMon) LOG_CONSOLE("%-22s @ %-3d days, %02d:%02d:%02d\n",
                                  "faultBatMon", faultBatMonTS / 86400, (faultBatMonTS % 86400) / 3600, (faultBatMonTS % 3600) / 60, (faultBatMonTS % 60));
  if (sFaultBMSSerialComms) LOG_CONSOLE("%-22s @ %-3d days, %02d:%02d:%02d\n",
                                          "faultBMSSerialComms", faultBMSSerialCommsTS / 86400, (faultBMSSerialCommsTS % 86400) / 3600, (faultBMSSerialCommsTS % 3600) / 60, (faultBMSSerialCommsTS % 60));
  if (sFaultBMSOV) LOG_CONSOLE("%-22s @ %-3d days, %02d:%02d:%02d\n",
                                 "faultBMSOV", faultBMSOVTS / 86400, (faultBMSOVTS % 86400) / 3600, (faultBMSOVTS % 3600) / 60, (faultBMSOVTS % 60));
  if (sFaultBMSUV) LOG_CONSOLE("%-22s @ %-3d days, %02d:%02d:%02d\n",
                                 "faultBMSUV", faultBMSUVTS / 86400, (faultBMSUVTS % 86400) / 3600, (faultBMSUVTS % 3600) / 60, (faultBMSUVTS % 60));
  if (sFaultBMSOT) LOG_CONSOLE("%-22s @ %-3d days, %02d:%02d:%02d\n",
                                 "faultBMSOT", faultBMSOTTS / 86400, (faultBMSOTTS % 86400) / 3600, (faultBMSOTTS % 3600) / 60, (faultBMSOTTS % 60));
  if (sFaultBMSUT) LOG_CONSOLE("%-22s @ %-3d days, %02d:%02d:%02d\n",
                                 "faultBMSUT", faultBMSUTTS / 86400, (faultBMSUTTS % 86400) / 3600, (faultBMSUTTS % 3600) / 60, (faultBMSUTTS % 60));
  if (sFault12VBatOV) LOG_CONSOLE("%-22s @ %-3d days, %02d:%02d:%02d\n",
                                    "fault12VBatOV", fault12VBatOVTS / 86400, (fault12VBatOVTS % 86400) / 3600, (fault12VBatOVTS % 3600) / 60, (fault12VBatOVTS % 60));
  if (sFault12VBatUV) LOG_CONSOLE("%-22s @ %-3d days, %02d:%02d:%02d\n",
                                    "fault12VBatUV", fault12VBatUVTS / 86400, (fault12VBatUVTS % 86400) / 3600, (fault12VBatUVTS % 3600) / 60, (fault12VBatUVTS % 60));
  if (sFaultWatSen1) LOG_CONSOLE("%-22s @ %-3d days, %02d:%02d:%02d\n",
                                   "faultWatSen1", faultWatSen1TS / 86400, (faultWatSen1TS % 86400) / 3600, (faultWatSen1TS % 3600) / 60, (faultWatSen1TS % 60));
  if (sFaultWatSen2) LOG_CONSOLE("%-22s @ %-3d days, %02d:%02d:%02d\n",
                                   "faultWatSen2", faultWatSen2TS / 86400, (faultWatSen2TS % 86400) / 3600, (faultWatSen2TS % 3600) / 60, (faultWatSen2TS % 60));
}
