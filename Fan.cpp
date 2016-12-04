#include <Arduino.h>
#include <PCF8574.h>

#include "Utils.cpp"

#ifndef FAN_CPP
#define FAN_CPP

class Fan {

private:
  PCF8574* expander;
  byte fanPwmPin;
  byte fanLedPin;
  
  byte power = 0;

public:
  Fan(PCF8574* expanderm, byte fanPwmPin, byte fanLedPin) { 
    this->expander = expander;
    this->fanPwmPin = fanPwmPin;
    this->fanLedPin = fanLedPin;
  }

  void changePower(byte targetPower) {
    if (targetPower == power) {
      return;
    }   
    
    if (targetPower > 0) {
      digitalWrite(fanLedPin, HIGH);  
    } else {
      digitalWrite(fanLedPin, LOW);    
    }
    
    if (targetPower > power) {
      for (byte i=max(power, 50); i<targetPower; i++) {
        analogWrite(fanPwmPin, i);
        delay(25);
      }
    } else {
      for (byte i=power; i>targetPower; i--) {
        analogWrite(fanPwmPin, i);
        delay(25);
      } 
    }

    power = targetPower;
  }
};

#endif
