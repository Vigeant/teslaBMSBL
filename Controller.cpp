#include "Controller.hpp"

/////////////////////////////////////////////////
/// \brief Orchestrates the activities within the BMS via a state machine.
/////////////////////////////////////////////////
void Controller::doController() {
  static int ticks = 0;
  static unsigned int bat12Vcyclestart = 0;
  const int stateticks = 4;
  float bat12vVoltage = (float)analogRead(INA_12V_BAT) / BAT12V_SCALING_DIVISOR ;

  if (state != INIT) syncModuleDataObjects();

  //figure out state transition
  switch (state) {

    case INIT:
      if (ticks >= stateticks) {
        ticks = 0;
        state = STANDBY;
      }
      break;

    case STANDBY:
#ifdef STATECYCLING
      if (ticks >= stateticks) {
        ticks = 0;
        state = STANDBY_DC2DC;
      }
#else
      if (digitalRead(INH_RUN) == HIGH) {
        ticks = 0;
        state = RUN;
      } else if (digitalRead(INL_EVSE_DISC) == HIGH) {
        ticks = 0;
        state = EVSE_CONNECTED;
      } else if (bat12vVoltage < DC2DC_CYCLE_V_SETPOINT) {
        ticks = 0;
        bat12Vcyclestart = (millis() / 1000);
        state = STANDBY_DC2DC;
      }
#endif
      break;

    case STANDBY_DC2DC:
#ifdef STATECYCLING
      if (ticks >= stateticks) {
        ticks = 0;
        state = EVSE_CONNECTED;
      }
#else
      if (digitalRead(INH_RUN) == HIGH) {
        ticks = 0;
        state = RUN;
      } else if (digitalRead(INL_EVSE_DISC) == HIGH) {
        ticks = 0;
        state = EVSE_CONNECTED_DC2DC;
      } else if (bat12Vcyclestart < (millis() / 1000)) {
        if ((bat12Vcyclestart + DC2DC_CYCLE_TIME_S) > (millis() / 1000)) {
          ticks = 0;
          state = STANDBY;
        }
        //if millis counter wrapped around
      } else if (bat12Vcyclestart > (millis() / 1000)) {
        if ((0xffffffff - bat12Vcyclestart + (millis() / 1000)) > DC2DC_CYCLE_TIME_S) {
          ticks = 0;
          state = STANDBY;
        }
      }
#endif
      break;

    case EVSE_CONNECTED:
#ifdef STATECYCLING
      if (ticks >= stateticks) {
        ticks = 0;
        state = EVSE_CONNECTED_DC2DC;
      }
#else
      if (digitalRead(INL_EVSE_DISC) == LOW) {
        ticks = 0;
        state = STANDBY;
      } else if (bms.getLowCellVolt() < CHARGER_CYCLE_V_SETPOINT) {
        ticks = 0;
        state = CHARGER_CYCLE;
      } else if (bat12vVoltage < DC2DC_CYCLE_V_SETPOINT) {
        ticks = 0;
        bat12Vcyclestart = (millis() / 1000);
        state = STANDBY_DC2DC;
      }
#endif
      break;

    case EVSE_CONNECTED_DC2DC:
#ifdef STATECYCLING
      if (ticks >= stateticks) {
        ticks = 0;
        state = CHARGER_CYCLE;
      }
#else
      if (digitalRead(INL_EVSE_DISC) == LOW) {
        ticks = 0;
        state = STANDBY_DC2DC;
      } else if (bms.getLowCellVolt() < CHARGER_CYCLE_V_SETPOINT) {
        ticks = 0;
        state = CHARGER_CYCLE;
      } else if (bat12Vcyclestart < (millis() / 1000)) {
        if ((bat12Vcyclestart + DC2DC_CYCLE_TIME_S) > (millis() / 1000)) {
          ticks = 0;
          state = EVSE_CONNECTED;
        }
        //if millis counter wrapped around
      } else if (bat12Vcyclestart > (millis() / 1000)) {
        if ((0xffffffff - bat12Vcyclestart + (millis() / 1000)) > DC2DC_CYCLE_TIME_S) {
          ticks = 0;
          state = EVSE_CONNECTED;
        }
      }
#endif
      break;

    case CHARGER_CYCLE:
      if (ticks >= stateticks) {
        ticks = 0;
        state = PRE_CHARGE;
      }
      break;

    case PRE_CHARGE:
#ifdef STATECYCLING
      if (ticks >= stateticks) {
        ticks = 0;
        state = CHARGING;
      }
#else
      if (digitalRead(INL_EVSE_DISC) == LOW) {
        ticks = 0;
        state = STANDBY;
      } else if (digitalRead(INH_CHARGING) == HIGH) {
        ticks = 0;
        state = CHARGING;
      }
#endif

      break;
    case CHARGING:
#ifdef STATECYCLING
      if (ticks >= stateticks) {
        ticks = 0;
        state = RUN;
      }
#else
      if (digitalRead(INL_EVSE_DISC) == LOW) {
        ticks = 0;
        state = STANDBY;
      } else if (digitalRead(INH_CHARGING) == LOW) {
        ticks = 0;
        state = EVSE_CONNECTED;
      }
#endif
      break;

    case RUN:
#ifdef STATECYCLING
      if (ticks >= stateticks) {
        ticks = 0;
        state = STANDBY;
      }
#else
      if (digitalRead(INH_RUN ) == LOW) {
        ticks = 0;
        state = STANDBY;
      }
#endif
      break;

    default:
      break;
  }

  

  //execute state
  switch (state) {

    case INIT:
      init();
      break;

    case STANDBY:
      standby();
      break;

    case STANDBY_DC2DC:
      standbyDC2DC();
      break;

    case EVSE_CONNECTED:
      evseConnected();
      break;

    case EVSE_CONNECTED_DC2DC:
      evseConnectedDC2DC();
      break;

    case CHARGER_CYCLE:
      cargerCycle();
      break;

    case PRE_CHARGE:
      preCharge();
      break;

    case CHARGING:
      charging();
      break;

    case RUN:
      run();
      break;

    default:
      break;
  }
  ticks++;
}

/////////////////////////////////////////////////
/// \brief When instantiated, the controller is in the init state ensuring that all the signal pins are set ptoperly.
/////////////////////////////////////////////////
Controller::Controller() {
  state = INIT;
}

/////////////////////////////////////////////////
/// \brief gathers all the data from the boards and check for any faulty state.
/////////////////////////////////////////////////
void Controller::syncModuleDataObjects() {
  float bat12vVoltage;
  bms.getAllVoltTemp();

  if (bms.getLineFault()) {
    if (!faultBMSSerialComms) {
      LOG_ERROR("Serial communication with battery modules lost!\n");
    }
    faultBMSSerialComms = true;
  } else {
    if (faultBMSSerialComms) LOG_INFO("Serial communication with battery modules re-established!\n");
    faultBMSSerialComms = false;
  }

  if (digitalRead(INL_BAT_PACK_FAULT) == LOW) {
    if (!faultModuleLoop) {
      LOG_ERROR("One or more BMS modules have asserted the fault loop!\n");
    }
    faultModuleLoop = true;
  } else {
    if (faultModuleLoop) LOG_INFO("All modules have deasserted the fault loop\n");
    faultModuleLoop = false;
  }

  if ( bms.getHighCellVolt() > OVER_V_SETPOINT ) {
    if (!faultBMSOV) {
      LOG_ERROR("OVER_V_SETPOINT: %.2fV, highest cell:%.2fV\n", OVER_V_SETPOINT, bms.getHighCellVolt());
    }
    faultBMSOV = true;
  } else {
    if (faultBMSOV) LOG_INFO("All cells are back under OV threshold\n");
    faultBMSOV = false;
  }

  if ( bms.getLowCellVolt() < UNDER_V_SETPOINT ) {
    if (!faultBMSUV) {
      LOG_ERROR("UNDER_V_SETPOINT: %.2fV, lowest cell:%.2fV\n", UNDER_V_SETPOINT, bms.getLowCellVolt());
    }
    faultBMSUV = true;
  } else {
    if (faultBMSUV) LOG_INFO("All cells are back over UV threshold\n");
    faultBMSUV = false;
  }

  if ( bms.getHighTemperature() > OVER_T_SETPOINT ) {
    if (!faultBMSOT) {
      LOG_ERROR("OVER_T_SETPOINT: %.2fV, highest module:%.2fV\n", UNDER_V_SETPOINT, bms.getHighTemperature());
    }
    faultBMSOT = true;
  } else {
    if (faultBMSOT) LOG_INFO("All modules are back under the OT threshold\n");
    faultBMSOT = false;
  }

  if ( bms.getLowTemperature() < UNDER_T_SETPOINT ) {
    if (!faultBMSUT) {
      LOG_ERROR("UNDER_T_SETPOINT: %.2fV, lowest module:%.2fV\n", UNDER_T_SETPOINT, bms.getLowTemperature());
    }
    faultBMSUT = true;
  } else {
    if (faultBMSUT) LOG_INFO("All modules are back over the UT threshold\n");
    faultBMSUT = false;
  }

  bat12vVoltage = (float)analogRead(INA_12V_BAT) / BAT12V_SCALING_DIVISOR ;
  //LOG_INFO("bat12vVoltage: %.2f\n", bat12vVoltage);

  if ( bat12vVoltage > BAT12V_OVER_V_SETPOINT ) {
    if (!fault12VBatOV) {
      LOG_ERROR("12VBAT_OVER_V_SETPOINT: %.2fV, V:%.2fV\n", BAT12V_OVER_V_SETPOINT, bat12vVoltage);
    }
    fault12VBatOV = true;
  } else {
    if (fault12VBatOV) LOG_INFO("12V battery back under the OV threshold\n");
    fault12VBatOV = false;
  }

  if ( bat12vVoltage < BAT12V_UNDER_V_SETPOINT ) {
    if (!fault12VBatUV) {
      LOG_ERROR("12VBAT_UNDER_V_SETPOINT: %.2fV, V:%.2fV\n", BAT12V_UNDER_V_SETPOINT, bat12vVoltage);
    }
    fault12VBatUV = true;
  } else {
    if (fault12VBatUV) LOG_INFO("12V battery back over the UV threshold\n");
    fault12VBatUV = false;
  }

  chargerInhibit = faultModuleLoop || faultBatMon || faultBMSSerialComms || faultBMSOV || faultBMSOT;
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

  stickyFaulted |= isFaulted;
  bms.clearFaults();
}

/////////////////////////////////////////////////
/// \brief balances the cells according to BALANCE_CELL_V_OFFSET threshold in the CONFIG.h file
/////////////////////////////////////////////////
void Controller::balanceCells() {
  //balance for 1 second given that the controller wakes up every second.
  bms.balanceCells(1);
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
  if (temp < COOLING_LOWT_SETPOINT) {
    return FLOOR_DUTY_COOLANT_PUMP;
  } else if (temp > COOLING_HIGHT_SETPOINT) {
    return 1.0;
  } else {
    return COOLING_A * temp + COOLING_B;
  }
}

/////////////////////////////////////////////////
/// \brief reset all boards and assign address to each board and configure their thresholds
/////////////////////////////////////////////////
void Controller::init() {
  pinMode(OUTL_12V_BAT_CHRG, OUTPUT);
  pinMode(OUTPWM_PUMP, OUTPUT); //PWM use analogWrite(OUTPWM_PUMP, 0-255);
  pinMode(INL_BAT_PACK_FAULT, INPUT_PULLUP);
  pinMode(INL_BAT_MON_FAULT, INPUT_PULLUP);
  pinMode(INL_EVSE_DISC, INPUT_PULLUP);
  pinMode(INH_RUN, INPUT_PULLDOWN);
  pinMode(INA_12V_BAT, INPUT);  // [0-1023] = analogRead(INA_12V_BAT)
  pinMode(OUTL_EVCC_ON, OUTPUT);
  pinMode(OUTL_NO_FAULT, OUTPUT);

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

  isFaulted = false;
  stickyFaulted = false;

  chargerInhibit = false;
  powerLimiter = false;

  bms.renumberBoardIDs();
  bms.clearFaults();
}

/////////////////////////////////////////////////
/// \brief standby state is when the boat *is not connected*
/// to a EVSE, not in run state and the 12V battery is above
/// its low voltage threshold.
/////////////////////////////////////////////////
void Controller::standby() {
  digitalWrite(OUTL_EVCC_ON, LOW);
  digitalWrite(OUTL_NO_FAULT, chargerInhibit);
  digitalWrite(OUTL_12V_BAT_CHRG, HIGH);
  analogWrite(OUTPWM_PUMP, 0);
}

/////////////////////////////////////////////////
/// \brief standbyDC2DC state is when the boat *is not connected*
/// to a EVSE, not in run state and the 12V battery is being
/// charged since it dipped bellow its low voltage threshold.
/////////////////////////////////////////////////
void Controller::standbyDC2DC() {
  digitalWrite(OUTL_EVCC_ON, LOW);
  digitalWrite(OUTL_NO_FAULT, chargerInhibit);
  digitalWrite(OUTL_12V_BAT_CHRG, LOW);
  analogWrite(OUTPWM_PUMP, 0);
}

/////////////////////////////////////////////////
/// \brief evseConnected state is when the boat *is connected*
/// to a EVSE and is not yet charging or in between 2 charging
/// cycles until it dips bellow its pack voltage threshold.
/////////////////////////////////////////////////
void Controller::evseConnected() {
  digitalWrite(OUTL_EVCC_ON, LOW);
  digitalWrite(OUTL_NO_FAULT, chargerInhibit);
  digitalWrite(OUTL_12V_BAT_CHRG, HIGH);
  analogWrite(OUTPWM_PUMP, 0);
}


/////////////////////////////////////////////////
/// \brief evseConnected state is when the boat *is connected*
/// to a EVSE and is not yet charging or in between 2 charging
/// cycles until it dips bellow its pack voltage threshold. 12V attery charging
/////////////////////////////////////////////////
void Controller::evseConnectedDC2DC() {
  digitalWrite(OUTL_EVCC_ON, LOW);
  digitalWrite(OUTL_NO_FAULT, chargerInhibit);
  digitalWrite(OUTL_12V_BAT_CHRG, LOW);
  analogWrite(OUTPWM_PUMP, 0);
}

/////////////////////////////////////////////////
/// \brief cargerCycle state is when the boat *is connected*
/// to a EVSE and is cycling the EVCC to trigger a new charging cycle.
/////////////////////////////////////////////////
void Controller::cargerCycle() {
  digitalWrite(OUTL_EVCC_ON, HIGH);
  digitalWrite(OUTL_NO_FAULT, chargerInhibit);
  digitalWrite(OUTL_12V_BAT_CHRG, HIGH);
  analogWrite(OUTPWM_PUMP, 0);
}

void Controller::preCharge() {
  digitalWrite(OUTL_EVCC_ON, LOW);
  digitalWrite(OUTL_NO_FAULT, chargerInhibit);
  digitalWrite(OUTL_12V_BAT_CHRG, HIGH);
  analogWrite(OUTPWM_PUMP, 0);
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
  analogWrite(OUTPWM_PUMP, getCoolingPumpDuty(bms.getHighTemperature()));
}

/////////////////////////////////////////////////
/// \brief run state is turned on and ready to operate.
/////////////////////////////////////////////////
void Controller::run() {
  digitalWrite(OUTL_EVCC_ON, HIGH);
  digitalWrite(OUTL_NO_FAULT, powerLimiter);
  digitalWrite(OUTL_12V_BAT_CHRG, LOW);
  analogWrite(OUTPWM_PUMP, getCoolingPumpDuty(bms.getHighTemperature()));
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
