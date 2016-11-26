#include <Arduino.h>

#ifndef LIGHT_SENSOR_CPP
#define LIGHT_SENSOR_CPP

class LightSensor {

private:
  byte fotoPin;
  boolean night = false;

public:
  LightSensor(byte fotoPin) { 
    this->fotoPin = fotoPin;
  }
  
  void update() {
    float fotoVoltage = analogRead(fotoPin) * 5.0 / 1023.0;
    if (fotoVoltage < 2) {
      night = true; 
    } else {
      night = false;
    }
  }

  boolean isNight() {
    return night;
  }
};

#endif
