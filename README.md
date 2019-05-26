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
| J | BMS Water Sensor 1 Fault |
| K | BMS Water Sensor 2 Fault |

## Connection to USB serial console

Serial Line: COMX (X typically = 7)
Speed: 115200
	
## todo
- [ ] Full state machine testing.

	
