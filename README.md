# teslaBMSBL

![Tesla BMS BL](misc/20190319_221311.jpg)
![Tesla BMS BL board](misc/20230725_143916.jpg)

## Hardware

see eagle files to build the board hosting the teensy 3.2.

## dependencies

- TeensyView libs
  - <https://github.com/sparkfun/SparkFun_TeensyView_Arduino_Library/tree/master/examples>
- Snooze for lower power consumption
  - <https://github.com/duff2013/Snooze>

## Error codes on teensyView

| code | definition |
|:----:|------------|
| A | Modules Fault Loop |
| B | Battery Monitor Fault |
| C | BMS Serial communication Fault |
| D | BMS Cell Over Voltage Fault |
| E | BMS Cell Under Voltage Fault |
| F | BMS Over Temperature Fault |
| G | BMS Under Temperature Fault |
| H | BMS 12V Battery Over Voltage Fault |
| I | BMS 12V Battery Under Voltage Fault |
| J | BMS Water Sensor 1 Fault |
| K | BMS Water Sensor 2 Fault |
| L | Incorrect modules count |

## Connection to USB serial console

Serial Line: COMX (X typically = 7)
Speed: 115200

## controller state machine

<!-- [State machine](https://online.visual-paradigm.com/w/pmcoivfe/diagrams.jsp#diagram:proj=0&id=3) -->

```mermaid
    stateDiagram-v2

    state "INIT

    outL_12V_bat_chrg_buffer = 1;
    outpwm_pump_buffer = 0;
    outL_evcc_on_buffer = 1;
    outH_fault_buffer = 0;" as INIT

    state "STANDBY

    balanceCells();
    outL_evcc_on_buffer = HIGH;
    outH_fault_buffer = chargerInhibit;
    outL_12V_bat_chrg_buffer = !dc2dcON_H;
    outpwm_pump_buffer = 0;" as STANDBY

    state "RUN

    outL_evcc_on_buffer = LOW;
    outH_fault_buffer = powerLimiter;
    outL_12V_bat_chrg_buffer = LOW;
    outpwm_pump_buffer = YES" as RUN

    state "PRE_CHARGE

    balanceCells();
    outL_evcc_on_buffer = LOW;
    outH_fault_buffer = chargerInhibit;
    outL_12V_bat_chrg_buffer = !dc2dcON_H;
    outpwm_pump_buffer = 0;
    msgStatusIns.bBMSStatusFlags = 0;" as PRE_CHARGE

    state "CHARGING

    balanceCells();
    outL_evcc_on_buffer = LOW;
    outH_fault_buffer = chargerInhibit;
    outL_12V_bat_chrg_buffer = LOW;
    outpwm_pump_buffer = YES;
    msgStatusIns.bBMSStatusFlags = 0;" as CHARGING

    state "TOP_BALANCING

    balanceCells();
    msgStatusIns.bBMSStatusFlags |= BMS_STATUS_CELL_BVC_FLAG;
    outL_evcc_on_buffer = LOW;
    outH_fault_buffer = chargerInhibit;
    outL_12V_bat_chrg_buffer = HIGH;
    outpwm_pump_buffer = YES;
    msgStatusIns.bBMSStatusFlags = BMS_STATUS_CELL_BVC_FLAG;" as TOP_BALANCING

    state "POST_CHARGE

    balanceCells();
    outL_evcc_on_buffer = HIGH;
    outH_fault_buffer = HIGH;
    outL_12V_bat_chrg_buffer = HIGH;
    outpwm_pump_buffer = YES;
    msgStatusIns.bBMSStatusFlags = BMS_STATUS_CELL_HVC_FLAG;" as POST_CHARGE

    [*] --> INIT

    INIT --> STANDBY

    STANDBY --> RUN : INH_RUN == HIGH
    STANDBY --> PRE_CHARGE : getHighCellVolt < charger_cycle_v_setpoint && \ngetHighCellVolt < max_charge_v_setpoint && \nticks >= standbyTicks
    STANDBY --> CHARGING : INH_CHARGING == HIGH

    PRE_CHARGE --> STANDBY : INL_EVSE_DISC == LOW
    PRE_CHARGE --> CHARGING :INH_CHARGING == HIGH
    PRE_CHARGE --> STANDBY : ticks > 100

    CHARGING --> POST_CHARGE : ticks >= 5 && \nINL_EVSE_DISC == LOW || \nINH_CHARGING == LOW
    CHARGING --> TOP_BALANCING : getHighCellVolt >= top_balance_v_setpoint

    TOP_BALANCING --> POST_CHARGE : ticks >= 5 && \nINL_EVSE_DISC == LOW || \nINH_CHARGING == LOW

    POST_CHARGE --> STANDBY : ticks >= 50 && \nINH_CHARGING == LOW

    RUN --> STANDBY : INH_RUN == LOW

```

## todo

- [X] none

## particularities

Due to the 5s deepsleep mode in standby, it is hard to connect the serial console. To make it easier, either connect within 10 minutes of a reset or place the bms in run mode and connect.
Due to the deepsleep mode, it is impossible to simply reprogram the teensy following the first sleep. To facilitate reprogramming, the board will not sleep for 10 minutes following a reset.
The teensyview cannot be shut down as it is connected straight to the VDD pins.
