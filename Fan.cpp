#include <Arduino.h>
#include <PCF8574.h>

#include "RelayModule.cpp"
#include "Utils.cpp"

#ifndef FAN_CPP
#define FAN_CPP

#define FAN_MODE_OFF 0
#define FAN_MODE_1 1
#define FAN_MODE_2 2
#define FAN_MODE_3 3

#define FAN_TEMP_SAMPLES 21
#define MIN_SAFE_FAN_TEMP 5
#define MAX_SAFE_FAN_TEMP_HIGH 40
#define MAX_SAFE_FAN_TEMP_LOW 38

class Fan {

private:
  RelayModule* relays;
  PCF8574* expander;

  byte fanMode = FAN_MODE_OFF;  

public:
  Fan(RelayModule* relays, PCF8574* expander) { 
    this->relays = relays;
    this->expander = expander;
  }

  byte getMode() {
    return fanMode;
  }

  void setMode(byte mode) {
    if (mode == fanMode) {
      return;
    } 
    
    relays->switchRelay(RELAY_3_3V, false);
    relays->switchRelay(RELAY_5V, false);
    relays->switchRelay(RELAY_12V, false);

    if (mode == FAN_MODE_OFF) {
      relays->switchRelay(RELAY_EXT_POWER, false);  
    } else if (mode < fanMode) {
      relays->switchRelay(RELAY_EXT_POWER, false);  
      delay(3000);
      relays->switchRelay(RELAY_EXT_POWER, true); 
    } else {
      relays->switchRelay(RELAY_EXT_POWER, true);      
    }

    delay(100);

    switch(mode) {
      case FAN_MODE_OFF:
        // nothing
        break;
      case FAN_MODE_1:
        relays->switchRelay(RELAY_3_3V, true);
        break;
      case FAN_MODE_2:
        relays->switchRelay(RELAY_5V, true);
        break;
      case FAN_MODE_3:
        relays->switchRelay(RELAY_5V, true);
        delay(1500);
        relays->switchRelay(RELAY_5V, false);
        delay(100);

        // soft start
        for (int i=1; i<=40; i++) {
          relays->switchRelay(RELAY_12V, true);  
          delay(i);
          relays->switchRelay(RELAY_12V, false);  
          delay(40-i);
        }
        
        relays->switchRelay(RELAY_12V, true); 
        
        break;
    }

    fanMode = mode; 
  }
};

#endif
