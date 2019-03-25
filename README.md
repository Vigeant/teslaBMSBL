# teslaBMSBL

![Tesla BMS BL](misc/20190319_221311.jpg)

## dependencies
- TeensyView libs
	- https://github.com/sparkfun/SparkFun_TeensyView_Arduino_Library/tree/master/examples
	
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
	
## todo
- [X] assign all signals to pins
- [X] Implement state machine
	- [X] noFault is driven differently from run to charging (run noFault is used to limit output while in charging it is used to prevent charging)
	- [X] add dc2dc states in charging mode to support shutting down dc2dc in diagram
	- [X] implement state machine in code to match diagram.
	- [X] update all states in the Oled class to match actual state machine.
- [X] update formula for pwm of cooling pump to floor at 25% duty cycle.

	
