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
    float pm10 = dustSensor->getPM10();
    float pm2_5 = dustSensor->getPM2_5();
    
    if (pm10 > PM_10_VERY_BAD || pm2_5 > PM_2_5_VERY_BAD) {
      setAirQuality(AIR_QUALITY_VERY_BAD);
      dustSensor->setSamplingMode(SAMPLING_MODE_AVG);
    } else if (pm10 > PM_10_BAD || pm2_5 > PM_2_5_BAD) {
      setAirQuality(AIR_QUALITY_BAD);
      dustSensor->setSamplingMode(SAMPLING_MODE_AVG);
    } else if (pm10 > PM_10_GOOD || pm2_5 > PM_2_5_GOOD) {
      setAirQuality(AIR_QUALITY_GOOD);
      dustSensor->setSamplingMode(SAMPLING_MODE_POWERSAVE);
    } else {
      setAirQuality(AIR_QUALITY_PERFECT);
      dustSensor->setSamplingMode(SAMPLING_MODE_POWERSAVE);
    }

    controlFan(pm10, pm2_5);
  }

private:
  
  void setAirQuality(byte level) {
    if (level == AIR_QUALITY_VERY_BAD) {
      rgbLed->red();
      if (airQualityLevel != AIR_QUALITY_VERY_BAD) {        
        buzzer->sound(80, 50, 4);
      }   
    } else if (level == AIR_QUALITY_BAD) {
      rgbLed->yellow();
      if (airQualityLevel != AIR_QUALITY_BAD) {        
        buzzer->sound(400, 50, 3);
      }   
    } else if (level == AIR_QUALITY_GOOD) {
      rgbLed->green();
      if (airQualityLevel != AIR_QUALITY_GOOD) {        
        buzzer->sound(1000, 50, 2);
      }   
    } else if (level == AIR_QUALITY_PERFECT) {
      rgbLed->blue();
      if (airQualityLevel != AIR_QUALITY_PERFECT) {        
        buzzer->sound(3000, 50, 1);
      }   
    }    
  
    airQualityLevel = level;
  }
  
  void controlFan(float pm10, float pm2_5) {
    float pm10Perc = pm10 / PM_10_VERY_BAD * 100.0; 
    float pm2_5Perc = pm2_5 / PM_2_5_VERY_BAD * 100.0; 
    float levelPerc = max(pm10Perc, pm2_5Perc);

    byte power = 0;
    
    if (levelPerc >= 100) {
      power = 255;  
    } else if (levelPerc > 50) {      
      power = levelPerc * 255/100;  
    }

    fan->changePower(power); 
  }
};

#endif
