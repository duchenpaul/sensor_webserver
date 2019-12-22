#include <ESP8266WiFi.h>                    //ESP8266 Core WiFi Library (you most likely already have this in your sketch)

uint8_t LEDpinR = D8;
uint8_t LEDpinG = D6;
uint8_t LEDpinB = D7;

 
/* By default PWM frequency is 1000Hz and we are using same 
   for this application hence no need to set */
 
void setup(){
  Serial.begin(115200);
  analogWrite(LEDpinR, 0);  /* set initial 50% duty cycle */
  analogWrite(LEDpinG, 0);  /* set initial 50% duty cycle */
  analogWrite(LEDpinB, 0);  /* set initial 50% duty cycle */

}
 
void loop(){
  uint16_t dutycycle =  analogRead(A0); /* read continuous POT and set PWM duty cycle according */
  if(dutycycle > 1023) dutycycle = 1023;/* limit dutycycle to 1023 if POT read cross it */
  // Serial.print("Duty Cycle: ");  Serial.println(dutycycle);
  analogWrite(LEDpinR, 1023 - dutycycle);
  analogWrite(LEDpinG, 1023 - dutycycle);
  analogWrite(LEDpinB, 1023 - dutycycle);
  delay(100);
}
