#include <Arduino.h>

#include "DustSensor.cpp"
#include "Fan.cpp"
#include "RGBLed.cpp"
#include "Buzzer.cpp"
#include "LightSensor.cpp"

#ifndef AIR_QUALITY_CONTROL_CPP
#define AIR_QUALITY_CONTROL_CPP

#define AIR_QUALITY_PERFECT 0
#define AIR_QUALITY_GOOD 1
#define AIR_QUALITY_BAD 2
#define AIR_QUALITY_VERY_BAD 3
#define AIR_QUALITY_UNKNOWN 255

#define PM_10_VERY_BAD 50
#define PM_10_BAD 25
#define PM_10_GOOD 15

#define PM_2_5_VERY_BAD 25
#define PM_2_5_BAD 15
#define PM_2_5_GOOD 8

class AirQualityControl {

private: 
  byte airQualityLevel = AIR_QUALITY_GOOD;  
  boolean autoMode = true;

  DustSensor* dustSensor;
  Fan* fan;
  RGBLed* rgbLed;
  Buzzer* buzzer;
  LightSensor* lightSensor;

public:
  AirQualityControl(DustSensor* dustSensor, Fan* fan, RGBLed* rgbLed, Buzzer* buzzer, LightSensor* lightSensor) { 
    this->dustSensor = dustSensor; 
    this->fan = fan;
    this->rgbLed = rgbLed;
    this->buzzer = buzzer;  
    this->lightSensor = lightSensor;  
  }

  void update() {
    int pm10 = dustSensor->getPM10();
    int pm2_5 = dustSensor->getPM2_5();
    
    if (pm10 > PM_10_VERY_BAD || pm2_5 > PM_2_5_VERY_BAD) {
      setAirQuality(AIR_QUALITY_VERY_BAD);
    } else if (pm10 > PM_10_BAD || pm2_5 > PM_2_5_BAD) {
      setAirQuality(AIR_QUALITY_BAD);
    } else if (pm10 > PM_10_GOOD || pm2_5 > PM_2_5_GOOD) {
      setAirQuality(AIR_QUALITY_GOOD);
    } else {
      setAirQuality(AIR_QUALITY_PERFECT);
    }
  }

  void setAutoMode(boolean enabled) {
    if (enabled) {
      if (!autoMode) {
        controlFanByAirQualityLevel(airQualityLevel);
      }
      autoMode = true; 
    } else {
      autoMode = false;
    }
  }

private:
  
  void setAirQuality(byte level) {
    if (level == AIR_QUALITY_VERY_BAD) {
      rgbLed->red();
      if (airQualityLevel != AIR_QUALITY_VERY_BAD && autoMode) {        
        buzzer->sound(80, 50, 4);
        controlFanByAirQualityLevel(level);
      }   
    } else if (level == AIR_QUALITY_BAD) {
      rgbLed->yellow();
      if (airQualityLevel != AIR_QUALITY_BAD && autoMode) {        
        buzzer->sound(400, 50, 3);
        controlFanByAirQualityLevel(level);
      }   
    } else if (level == AIR_QUALITY_GOOD) {
      rgbLed->green();
      if (airQualityLevel != AIR_QUALITY_GOOD && autoMode) {        
        buzzer->sound(1000, 50, 2);
        controlFanByAirQualityLevel(level);
      }   
    } else if (level == AIR_QUALITY_PERFECT) {
      rgbLed->blue();
      if (airQualityLevel != AIR_QUALITY_PERFECT && autoMode) {        
        buzzer->sound(3000, 50, 1);
        controlFanByAirQualityLevel(level);
      }   
    }    
  
    airQualityLevel = level;
  }
  
  void controlFanByAirQualityLevel(byte level) {
    switch (level) {
      case AIR_QUALITY_PERFECT:
        fan->setMode(FAN_MODE_OFF);
        break;
      case AIR_QUALITY_GOOD:
        fan->setMode(FAN_MODE_1);
        break;
      case AIR_QUALITY_BAD:
        fan->setMode(FAN_MODE_2);
        break;
      case AIR_QUALITY_VERY_BAD:
        fan->setMode(FAN_MODE_3);
        break;
    }
  }
};

#endif
