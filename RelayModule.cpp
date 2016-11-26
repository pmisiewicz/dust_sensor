#include <Arduino.h>
#include <PCF8574.h>

#ifndef RELAY_MODULE_CPP
#define RELAY_MODULE_CPP

#define RELAY_3_3V 1
#define RELAY_5V 2
#define RELAY_12V 3
#define RELAY_EXT_POWER 4

class RelayModule {

private:
  PCF8574* expander;  
  byte relay1Pin;
  byte relay2Pin;
  byte relay3Pin;
  byte relay4Pin;

public:
  RelayModule(PCF8574* expander, byte relay1Pin, byte relay2Pin, byte relay3Pin, byte relay4Pin) { 
    this->relay1Pin = relay1Pin;  
    this->relay2Pin = relay2Pin;
    this->relay3Pin = relay3Pin;
    this->relay4Pin = relay4Pin;
  }
  
  void switchRelay(byte relay, boolean on) {
    switch (relay) {
       case RELAY_3_3V: 
          digitalWrite(relay1Pin, on ? HIGH : LOW);
          break; 
       case RELAY_5V: 
          digitalWrite(relay2Pin, on ? HIGH : LOW);
          break; 
       case RELAY_12V: 
          digitalWrite(relay3Pin, on ? HIGH : LOW);
          break;
        case RELAY_EXT_POWER: 
          digitalWrite(relay4Pin, on ? HIGH : LOW);
          break;  
    }
  }
};

#endif
