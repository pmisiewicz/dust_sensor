#include <Arduino.h>
#include <PCF8574.h>

#include "Utils.cpp"

#ifndef FAN_CPP
#define FAN_CPP

#define FAN_MODE_OFF 0
#define FAN_MODE_1 1
#define FAN_MODE_2 2
#define FAN_MODE_3 3

class Fan {

private:
  PCF8574* expander;
  byte fanPwmPin;
  byte fanLedPin;
  
  byte fanMode = FAN_MODE_OFF;  
  byte power = 0;

public:
  Fan(PCF8574* expanderm, byte fanPwmPin, byte fanLedPin) { 
    this->expander = expander;
    this->fanPwmPin = fanPwmPin;
    this->fanLedPin = fanLedPin;
  }

  byte getMode() {
    return fanMode;
  }

  void setMode(byte mode) {
    if (mode == fanMode) {
      return;
    } 
    
    switch(mode) {
      case FAN_MODE_OFF:
        changePower(0);
        break;
      case FAN_MODE_1:
        changePower(95);
        break;
      case FAN_MODE_2:
        changePower(185);
        break;
      case FAN_MODE_3:
        changePower(255);
        break;
    }

    fanMode = mode; 
  }

  void changePower(byte targetPower) {
    if (targetPower > 0) {
      digitalWrite(fanLedPin, HIGH);  
    } else {
      digitalWrite(fanLedPin, LOW);    
    }
    
    if (targetPower > power) {
      for (byte i=max(power, 50); i<targetPower; i++) {
        analogWrite(fanPwmPin, i);
        delay(50);
      }
    } else {
      for (byte i=power; i>targetPower; i--) {
        analogWrite(fanPwmPin, i);
        delay(50);
      } 
    }

    power = targetPower;
  }
};

#endif
