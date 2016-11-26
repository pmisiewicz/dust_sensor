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

// http://monitoring.krakow.pios.gov.pl/indeks-jakosci-powietrza
#define PM_10_VERY_BAD 50
#define PM_10_VERY_BAD_LOW 45
#define PM_10_BAD 25
#define PM_10_BAD_LOW 20
#define PM_10_GOOD 15
#define PM_10_GOOD_LOW 10

#define PM_2_5_VERY_BAD 25
#define PM_2_5_VERY_BAD_LOW 20
#define PM_2_5_BAD 15
#define PM_2_5_BAD_LOW 10
#define PM_2_5_GOOD 8
#define PM_2_5_GOOD_LOW 5

class AirQualityControl {

private: 
  byte airQualityLevel = AIR_QUALITY_GOOD;  
  boolean autoMode = true;

  DustSensor* dustSensor;
  Fan* fan;
  RGBLed* rgbLed;
  Buzzer* buzzer;
  LightSensor* lightSensor;
  byte autoModeLedPin;  

public:
  AirQualityControl(DustSensor* dustSensor, Fan* fan, RGBLed* rgbLed, Buzzer* buzzer, LightSensor* lightSensor, byte autoModeLedPin) { 
    this->dustSensor = dustSensor; 
    this->fan = fan;
    this->rgbLed = rgbLed;
    this->buzzer = buzzer;  
    this->lightSensor = lightSensor;  
    this->autoModeLedPin = autoModeLedPin;
  }

  void update() {
    int pm10 = dustSensor->getPM10();
    int pm2_5 = dustSensor->getPM2_5();
    
    if (pm10 > PM_10_VERY_BAD || pm2_5 > PM_2_5_VERY_BAD) {
      setAirQuality(AIR_QUALITY_VERY_BAD);
    } else if (pm10 > PM_10_BAD || pm2_5 > PM_2_5_BAD) {
      if (airQualityLevel <= AIR_QUALITY_BAD || (pm10 <= PM_10_VERY_BAD_LOW && pm2_5 <= PM_2_5_VERY_BAD_LOW)) {
        setAirQuality(AIR_QUALITY_BAD);
      } else {
        setAirQuality(airQualityLevel);  
      }
    } else if (pm10 > PM_10_GOOD || pm2_5 > PM_2_5_GOOD) {
      if (airQualityLevel <= AIR_QUALITY_GOOD || (pm10 <= PM_10_BAD_LOW && pm2_5 <= PM_2_5_BAD_LOW)) {
        setAirQuality(AIR_QUALITY_GOOD);
      } else {
        setAirQuality(airQualityLevel);  
      }
    } else {
      if (airQualityLevel <= AIR_QUALITY_PERFECT || (pm10 <= PM_10_GOOD_LOW && pm2_5 <= PM_2_5_GOOD_LOW)) {
        setAirQuality(AIR_QUALITY_PERFECT);
      } else {
        setAirQuality(airQualityLevel);  
      }
    }

    if (autoMode && !lightSensor->isNight()) {
      digitalWrite(autoModeLedPin, HIGH);   
    } else {
      digitalWrite(autoModeLedPin, LOW);   
    }
  }

  void setAutoMode(boolean enabled) {
    if (enabled) {
      if (!autoMode) {
        controlFanByAirQualityLevel(airQualityLevel);
      }
      autoMode = true; 
      digitalWrite(autoModeLedPin, HIGH); 
    } else {
      autoMode = false;
      digitalWrite(autoModeLedPin, LOW);
    }
  }

private:
  
  void setAirQuality(byte level) {
    if (level == AIR_QUALITY_VERY_BAD) {
      rgbLed->red();
      if (airQualityLevel != AIR_QUALITY_VERY_BAD && autoMode) {
        controlFanByAirQualityLevel(level);
        buzzer->sound(80, 50, 4);
      }   
    } else if (level == AIR_QUALITY_BAD) {
      rgbLed->yellow();
      if (airQualityLevel != AIR_QUALITY_BAD && autoMode) {
        controlFanByAirQualityLevel(level);
        buzzer->sound(400, 50, 3);
      }   
    } else if (level == AIR_QUALITY_GOOD) {
      rgbLed->green();
      if (airQualityLevel != AIR_QUALITY_GOOD && autoMode) {
        controlFanByAirQualityLevel(level);
        buzzer->sound(1000, 50, 2);
      }   
    } else if (level == AIR_QUALITY_PERFECT) {
      rgbLed->blue();
      if (airQualityLevel != AIR_QUALITY_PERFECT && autoMode) {
        controlFanByAirQualityLevel(level);
        buzzer->sound(3000, 50, 1);
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
