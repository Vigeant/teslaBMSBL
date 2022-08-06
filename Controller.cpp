#include "Controller.hpp"

/////////////////////////////////////////////////
/// \brief Orchestrates the activities within the BMS via a state machine.
/////////////////////////////////////////////////
void Controller::doController() {
  static int ticks = 0;
  static unsigned int bat12Vcyclestart = 0;
  static int standbyTicks = 1; //1 because ticks slow down
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
        LOG_INFO("Transition to STANDBY\n");
      }
      break;
    /**************** ****************/
    case STANDBY:
#ifdef STATECYCLING
      if (ticks >= stateticks) {
        ticks = 0;
        state = PRE_CHARGE;
        LOG_INFO("Transition to PRE_CHARGE\n");
      }
#else
      if (dc2dcON_H == 0 && bat12vVoltage < DC2DC_CYCLE_V_SETPOINT) {
        bat12Vcyclestart = (millis() / 1000);
        dc2dcON_H = 1;
      } else if ( dc2dcON_H == 1 && ((millis() / 1000) > bat12Vcyclestart + DC2DC_CYCLE_TIME_S)) {
        // I am ignoring the time counter loop around as it will simply result in a short charge cycle
        // followed with a proper charging cycle.
        dc2dcON_H = 0;
      }

      if (digitalRead(INH_RUN) == HIGH) {
        ticks = 0;
        state = RUN;
        LOG_INFO("Transition to RUN\n");
      } else if (digitalRead(INH_CHARGING) == HIGH) {
        ticks = 0;
        state = CHARGING;
        LOG_INFO("Transition to CHARGING\n");
      } else if (bms.getHighCellVolt() < CHARGER_CYCLE_V_SETPOINT && bms.getHighCellVolt() < MAX_CHARGE_V_SETPOINT && ticks >= standbyTicks) {
        ticks = 0;
        state = PRE_CHARGE;
        LOG_INFO("Transition to PRE_CHARGE\n");
      }
#endif
      break;
    /**************** ****************/
    case PRE_CHARGE:
#ifdef STATECYCLING
      if (ticks >= stateticks) {
        ticks = 0;
        state = CHARGING;
        LOG_INFO("Transition to CHARGING\n");
      }
#else
      if (ticks >= 50 && ticks <= 100) { //adjust to give time to the EVCC to properly boot (5 ticks == 1 seconds)
        if ( (digitalRead(INL_EVSE_DISC) == LOW) || (digitalRead(INH_CHARGING) == LOW) ) {
          ticks = 0;
          state = STANDBY;
          LOG_INFO("Transition to STANDBY\n");
        } else if (digitalRead(INH_CHARGING) == HIGH) {
          ticks = 0;
          state = CHARGING;
          LOG_INFO("Transition to CHARGING\n");
        }
      } else if (ticks > 100) {
        ticks = 0;
        state = STANDBY;
        LOG_ERR("charger did not start!!!\n");
        LOG_INFO("Transition to STANDBY\n");
      }
#endif
      break;
    /**************** ****************/
    case CHARGING:
#ifdef STATECYCLING
      if (ticks >= stateticks) {
        ticks = 0;
        state = TRICKLE_CHARGING;
        LOG_INFO("Transition to TRICKLE_CHARGING\n");
      }
#else
      if (ticks >= 5 && (digitalRead(INL_EVSE_DISC) == LOW || digitalRead(INH_CHARGING) == LOW)) {
        ticks = 0;
        state = POST_CHARGE;
        LOG_INFO("Transition to POST_CHARGE\n");
      } else if (digitalRead(INL_EVSE_DISC) == LOW || digitalRead(INH_CHARGING) == LOW) {
        //debounce error by letting ticks go up to 5.
        LOG_INFO("INL_EVSE_DISC == LOW || INH_CHARGING == LOW\n");
      } else if (bms.getHighCellVolt() > TRICKLE_CHARGE_V_SETPOINT) {
        ticks = 0;
        state = TRICKLE_CHARGING;
        LOG_INFO("Transition to TRICKLE_CHARGING\n");
      } else {
        ticks = 0;
      }
#endif
      break;
    /**************** ****************/
    case TRICKLE_CHARGING:
#ifdef STATECYCLING
      if (ticks >= stateticks) {
        ticks = 0;
        state = POST_CHARGE;
        LOG_INFO("Transition to POST_CHARGE\n");
      }
#else
      if (ticks >= 5 && (digitalRead(INL_EVSE_DISC) == LOW || digitalRead(INH_CHARGING) == LOW)) {
        ticks = 0;
        state = POST_CHARGE;
        LOG_INFO("Transition to POST_CHARGE\n");
      } else if (digitalRead(INL_EVSE_DISC) == LOW || digitalRead(INH_CHARGING) == LOW) {
        //debounce error by letting ticks go up to 5.
        LOG_INFO("INL_EVSE_DISC == LOW || INH_CHARGING == LOW\n");
      } else {
        ticks = 0;
      }
#endif
      break;
    /**************** ****************/
    case POST_CHARGE:
#ifdef STATECYCLING
      if (ticks >= stateticks) {
        ticks = 0;
        state = RUN;
        LOG_INFO("Transition to RUN\n");
      }
#else
      //adjust to give time to the EVCC to properly go to sleep (5 ticks == 1 seconds)
      //This will happen if the loop evcc input (fault line) is asserted.
      if (ticks >= 50 && digitalRead(INH_CHARGING) == LOW) {
        ticks = 0;
        state = STANDBY;
        LOG_INFO("Transition to STANDBY\n");
      }
#endif
      break;
    /**************** ****************/
    case RUN:
#ifdef STATECYCLING
      if (ticks >= stateticks) {
        ticks = 0;
        state = INIT;
        LOG_INFO("Transition to INIT\n");
      }
#else
      if (digitalRead(INH_RUN ) == LOW) {
        ticks = 0;
        state = STANDBY;
        LOG_INFO("Transition to STANDBY\n");
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
      period = LOOP_PERIOD_ACTIVE_MS;
      init();
      break;

    case STANDBY:
      //prevents sleeping if the console is connected or if within 10 minutes of a hard reset.
      //The teensy wont let reprogram if it slept once so this allows reprograming within 10 minutes.
      if (SERIALCONSOLE || millis() < 600000) {
        period = LOOP_PERIOD_ACTIVE_MS;
        standbyTicks = 10;
      } else {
        period = LOOP_PERIOD_STANDBY_MS;
        standbyTicks = 1;
      }
      standby();
      break;

    case PRE_CHARGE:
      period = LOOP_PERIOD_ACTIVE_MS;
      pre_charge();
      break;

    case CHARGING:
      period = LOOP_PERIOD_ACTIVE_MS;
      charging();
      break;

    case TRICKLE_CHARGING:
      period = LOOP_PERIOD_ACTIVE_MS;
      trickle_charging();
      break;

    case POST_CHARGE:
      period = LOOP_PERIOD_ACTIVE_MS;
      post_charge();
      break;

    case RUN:
      period = LOOP_PERIOD_ACTIVE_MS;
      run();
      break;

    default:
      period = LOOP_PERIOD_ACTIVE_MS;
      break;
  }
  ticks++;
  //set outputs
  setOutput(OUTL_EVCC_ON, outL_evcc_on_buffer);
  setOutput(OUTH_FAULT, outH_fault_buffer);
  setOutput(OUTL_12V_BAT_CHRG, outL_12V_bat_chrg_buffer);
  analogWrite(OUTPWM_PUMP, outpwm_pump_buffer);
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

  //added bms.getHighCellVolt() >= MAX_CHARGE_V_SETPOINT to stop charging even if charging in run state.
  chargerInhibit = faultModuleLoop || faultBatMon || faultBMSSerialComms || faultBMSOV || faultBMSUT || faultBMSOT || faultWatSen1 || faultWatSen2;
  chargerInhibit |= bms.getHighCellVolt() >= MAX_CHARGE_V_SETPOINT;
  powerLimiter   = faultModuleLoop || faultBatMon || faultBMSSerialComms || faultBMSOV || faultBMSUT || faultBMSOT || faultWatSen1 || faultWatSen2 || faultBMSUV;
  powerLimiter |= bms.getHighCellVolt() >= MAX_CHARGE_V_SETPOINT;
  //powerLimiter = faultModuleLoop || faultBatMon || faultBMSSerialComms || faultBMSUV || faultBMSOT;
  isFaulted =  chargerInhibit || faultBMSUV || faultBMSUT || fault12VBatOV || fault12VBatUV;

  if (bms.getHighCellVolt() >= MAX_CHARGE_V_SETPOINT) LOG_INFO("bms.getHighCellVolt()[%.2f] >= MAX_CHARGE_V_SETPOINT", bms.getHighCellVolt());
  if (chargerInhibit) LOG_INFO("chargerInhibit (fault) line asserted!\n");
  if (powerLimiter) LOG_INFO("powerLimiter (fault) line asserted!\n");

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
///
/// Returns a float from 0.0 - 1.0 that must be adjusted to the PWM range (0-255).
///
/// @param the temparature in C
/////////////////////////////////////////////////
//pwd = a*temp + b
#define COOLING_A (1.0 - FLOOR_DUTY_COOLANT_PUMP) / (COOLING_HIGHT_SETPOINT - COOLING_LOWT_SETPOINT)
#define COOLING_B FLOOR_DUTY_COOLANT_PUMP - COOLING_A * COOLING_LOWT_SETPOINT
float Controller::getCoolingPumpDuty(float temp) {
  //LOG_INFO("Cooling Pump Set To Max duty\n");
  return 1.0; //always fully on.
  /*
    if (temp < COOLING_LOWT_SETPOINT) {
    return FLOOR_DUTY_COOLANT_PUMP;
    } else if (temp > COOLING_HIGHT_SETPOINT) {
    return 1.0;
    } else {
    return COOLING_A * temp + COOLING_B;
    }
  */
}

/////////////////////////////////////////////////
/// \brief reset all boards, assign address to each board and configure their thresholds
/////////////////////////////////////////////////
void Controller::init() {
  pinMode(OUTL_12V_BAT_CHRG, INPUT);
  pinMode(OUTPWM_PUMP, OUTPUT); //PWM use analogWrite(OUTPWM_PUMP, 0-255);
  pinMode(INL_BAT_PACK_FAULT, INPUT_PULLUP);
  pinMode(INL_BAT_MON_FAULT, INPUT_PULLUP);
  pinMode(INL_EVSE_DISC, INPUT_PULLUP);
  pinMode(INH_RUN, INPUT_PULLDOWN);
  pinMode(INH_CHARGING, INPUT_PULLDOWN);
  pinMode(INA_12V_BAT, INPUT);  // [0-1023] = analogRead(INA_12V_BAT)
  pinMode(OUTL_EVCC_ON, OUTPUT);
  pinMode(OUTH_FAULT, OUTPUT);
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
  dc2dcON_H = false;
  period = LOOP_PERIOD_ACTIVE_MS;

  outL_12V_bat_chrg_buffer = 1;
  outpwm_pump_buffer = 0;
  outL_evcc_on_buffer = 1;
  outH_fault_buffer = 0;

  bms.renumberBoardIDs();
  bms.clearFaults();
}

/////////////////////////////////////////////////
/// \brief This helper function allows mimicking an open collector output (floating or ground).
/////////////////////////////////////////////////
void Controller::setOutput(int pin, int state) {
  if (state == 1) {
    pinMode(pin, INPUT);
  } else {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, state);
  }
}
/////////////////////////////////////////////////
/// \brief standby state is when the boat is not charging and not in run state.
/////////////////////////////////////////////////
void Controller::standby() {
  balanceCells();
  outL_evcc_on_buffer = HIGH;
  outH_fault_buffer = chargerInhibit;
  outL_12V_bat_chrg_buffer = !dc2dcON_H;
  outpwm_pump_buffer = 0;
}

/////////////////////////////////////////////////
/// \brief pre_charge state is turning on the EVCC to charge the battery. If the EVSE is disconnected,
/// it goes back to STANDBY.
/////////////////////////////////////////////////
void Controller::pre_charge() {
  balanceCells();
  outL_evcc_on_buffer = LOW;
  outH_fault_buffer = chargerInhibit;
  outL_12V_bat_chrg_buffer = !dc2dcON_H;
  outpwm_pump_buffer = 0;
}

/////////////////////////////////////////////////
/// \brief charging state is when the boat *is connected*
/// to a EVSE and is actively charging until EVCC shuts itself down.
/////////////////////////////////////////////////
void Controller::charging() {
  balanceCells();
  outL_evcc_on_buffer = LOW;
  outH_fault_buffer = chargerInhibit;
  outL_12V_bat_chrg_buffer = LOW;
  outpwm_pump_buffer = (uint8_t) (getCoolingPumpDuty(bms.getHighTemperature()) * 255 );
}

/////////////////////////////////////////////////
/// \brief trickle_charging state is when the boat *is connected*
/// to a EVSE and is finishing up charging until EVCC shuts itself down.
/// During this phase, we turn off the DC2DC so as not to confuse the EVCC
/// as it tries to measure a current drop in the charging.
/////////////////////////////////////////////////
void Controller::trickle_charging() {
  balanceCells();
  outL_evcc_on_buffer = LOW;
  outH_fault_buffer = chargerInhibit;
  outL_12V_bat_chrg_buffer = HIGH; //stop DC2DC
  outpwm_pump_buffer = (uint8_t) (getCoolingPumpDuty(bms.getHighTemperature()) * 255 );
}

/////////////////////////////////////////////////
/// \brief post_charging state is when the boat is finishing a charge
/// to to let the EVCC go back to sleep.
/////////////////////////////////////////////////
void Controller::post_charge() {
  balanceCells();
  outL_evcc_on_buffer = HIGH;      // de-assert key switch ignition KSI to the EVCC
  outH_fault_buffer = HIGH; //keep asserting the loop to the EVCC until it goes to sleep
  outL_12V_bat_chrg_buffer = HIGH; //stop DC2DC
  outpwm_pump_buffer = (uint8_t) (getCoolingPumpDuty(bms.getHighTemperature()) * 255 );
}
/////////////////////////////////////////////////
/// \brief run state is turned on and ready to operate.
/////////////////////////////////////////////////
void Controller::run() {
  outL_evcc_on_buffer = LOW;      //required so that the EVSE_DISC is valid (will inhibit the motor controller if EVSE is connected)
  outH_fault_buffer = powerLimiter;
  outL_12V_bat_chrg_buffer = LOW;
  outpwm_pump_buffer = (uint8_t) (getCoolingPumpDuty(bms.getHighTemperature()) * 255 );
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
  LOG_CONSOLE("OUTL_EVCC_ON: %d\n", outL_evcc_on_buffer);
  LOG_CONSOLE("OUTH_FAULT: %d\n", outH_fault_buffer);
  LOG_CONSOLE("OUTL_12V_BAT_CHRG: %d\n", outL_12V_bat_chrg_buffer);
  LOG_CONSOLE("OUTPWM_PUMP: %d\n", outpwm_pump_buffer);
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
    case TRICKLE_CHARGING:
      LOG_CONSOLE("=  state: TRICKLE_CHARGING                                                         =\n");
      break;
    case POST_CHARGE:
      LOG_CONSOLE("=  state: POST_CHARGE                                                              =\n");
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
