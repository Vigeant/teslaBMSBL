/**@file teslaBMSBL.ino */
#include <Arduino.h>
#include "CONFIG.h"

#define CPU_RESTART_ADDR (uint32_t *)0xE000ED0C
#define CPU_RESTART_VAL 0x5FA0004
#define CPU_RESTART (*CPU_RESTART_ADDR = CPU_RESTART_VAL);


char buffer[200];

void setup() {
  Serial.begin(115200);
  //Serial.begin(9600);
  Serial.setTimeout(15);
  sprintf (buffer, "[!] Configuring pin %d as OUTPUT\n", OUTPWM_PUMP);
  Serial.print (buffer);
  pinMode(OUTPWM_PUMP, OUTPUT); //PWM use analogWrite(OUTPWM_PUMP, 0-255);
  sprintf (buffer,"[+] Configured pin %d as OUTPUT\n", OUTPWM_PUMP);
  Serial.print (buffer);
}

void loop()
{
  unsigned char pumpduty = 0;
  for (;;) {
    analogWrite(OUTPWM_PUMP, pumpduty);
    sprintf (buffer,"[+] pumpduty = %d\n", pumpduty++);
    Serial.print (buffer);
    delay(100);
  }
}
