#include <Arduino.h>
#include <PCF8574.h>

#include "TemperatureSensor.cpp"
#include "Utils.cpp"

#ifndef DUST_SENSOR_CPP
#define DUST_SENSOR_CPP

#define LENG 31  // 0x42 + 31 bytes equal to 32 bytes

#define DUST_SAMPLES_CNT 50

#define SAMPLING_MODE_AVG 1
#define SAMPLING_MODE_POWERSAVE 2

#define DEFAULT_SAMPLING_MODE SAMPLING_MODE_AVG

#define SAMPLING_MODE_POWERSAVE_COLLECT_MILLIS 1000ul * 60ul * 1ul
#define SAMPLING_MODE_POWERSAVE_SLEEP_MILLIS 1000ul * 60ul * 5ul

#define IGNORED_SAMPLES_ON_START 10

class DustSensor {
  
private:
 
  unsigned char buf[LENG];

  float currentDustLevelPM2_5 = 0;
  float currentDustLevelPM10 = 0;

  int dustSampleIdx = 0;
  
  int* pm2_5Samples = new int[DUST_SAMPLES_CNT];
  int* pm10Samples = new int[DUST_SAMPLES_CNT];

  byte samplingMode = DEFAULT_SAMPLING_MODE;

  boolean powerSaveCollecting = false;
  unsigned long powerSaveLastCollectingTime = 0;

  boolean firstRun = true;
  byte ignoredSamples = 0;  

  boolean sleeping = false;

  PCF8574* expander;
  TemperatureSensor* temperatureSensor;
  byte sleepPin;

public:

  DustSensor(PCF8574* expander, TemperatureSensor* temperatureSensor, byte sleepPin) {
    this->expander = expander;
    this->temperatureSensor = temperatureSensor;
    this->sleepPin = sleepPin;

    for (int i = 0; i < DUST_SAMPLES_CNT; i++) {
      pm2_5Samples[i] = -1;
      pm10Samples[i] = -1;
    }
  }

  float getPM2_5() { 
    return currentDustLevelPM2_5; 
  }

  float getPM10() { 
    return currentDustLevelPM10; 
  }

  void sleep() { 
    expander->digitalWrite(sleepPin, HIGH); 
    sleeping = true;
  }

  void wake() { 
    expander->digitalWrite(sleepPin, LOW); 
    sleeping = false;
  }

  void setSamplingMode(byte targetMode) {
    if (samplingMode == targetMode) {
      return;
    }

    samplingMode = targetMode; 
    powerSaveCollecting = false;
    powerSaveLastCollectingTime = 0;
    ignoredSamples = 0;
    wake();        
  }

  void update() {
    if (samplingMode == SAMPLING_MODE_POWERSAVE) {
      if (!powerSaveCollecting && powerSaveLastCollectingTime < timeMillis() - SAMPLING_MODE_POWERSAVE_SLEEP_MILLIS) {
        powerSaveCollecting = true;
        powerSaveLastCollectingTime = timeMillis();
        ignoredSamples = 0;
        wake();
      } else if (powerSaveCollecting && powerSaveLastCollectingTime < timeMillis() - SAMPLING_MODE_POWERSAVE_COLLECT_MILLIS) {
        powerSaveCollecting = false;
        powerSaveLastCollectingTime = timeMillis();
        ignoredSamples = 0;
        firstRun = false;
        sleep();
      } else if (!powerSaveCollecting) {
        sleep();
      }
    } else {
      wake();
    }
  }

  boolean isSleeping() {
    return sleeping;
  }

  void onSerialEvent() {
    if (Serial.find(0x42)) {
      Serial.readBytes(buf, LENG);

      if (!firstRun && ignoredSamples < IGNORED_SAMPLES_ON_START) {
        ignoredSamples++;
        return; 
      }

      if (buf[0] == 0x4d) {
        if (checkValue(buf, LENG)) {
          int PM01Value = transmitPM01(buf);
          int PM2_5Value = transmitPM2_5(buf);
          int PM10Value = transmitPM10(buf);
          calculateDustLevels(PM01Value, PM2_5Value, PM10Value);
        }
      }
    }
  }

private:
 
  void calculateDustLevels(int PM01Value, int PM2_5Value, int PM10Value) {
    int humidity = temperatureSensor->lastReading().h;

    PM2_5Value = numericCorrectionPM2_5(PM2_5Value, humidity);
    PM10Value = numericCorrectionPM10(PM10Value, humidity);

    pm2_5Samples[dustSampleIdx] = PM2_5Value;
    pm10Samples[dustSampleIdx] = PM10Value;
    
    dustSampleIdx++;
    dustSampleIdx = dustSampleIdx % DUST_SAMPLES_CNT;

    float sumDustLevelPM2_5 = 0;
    float sumDustLevelPM10 = 0;
    int validSamplesCnt = 0;

    for (int i = 0; i < DUST_SAMPLES_CNT; i++) {
      if (pm2_5Samples[i] >= 0 && pm10Samples[i] >= 0) {
        sumDustLevelPM10 += pm10Samples[i];
        sumDustLevelPM2_5 += pm2_5Samples[i];
        validSamplesCnt++;
      }
    }

    currentDustLevelPM2_5 = sumDustLevelPM2_5 / validSamplesCnt;
    currentDustLevelPM10 = sumDustLevelPM10 / validSamplesCnt;   
  }
 
  float numericCorrectionPM2_5(float raw, float humidity) {
    double cpa = 1.00123;
    double cpb = 1.96141;
    double cpc = 1.0;
    double divider = cpc + cpa * pow((humidity / 100.0), cpb);
    return raw / divider + 0.5;  
  }

  float numericCorrectionPM10(float raw, float humidity) {
    double cpa = 1.15866;
    double cpb = 3.16930;
    double cpc = 0.7;
    double divider = cpc + cpa * pow((humidity / 100.0), cpb);
    return raw / divider + 0.5;   
  }
 
  char checkValue(unsigned char* thebuf, char leng) {
    char receiveflag = 0;
    int receiveSum = 0;

    for (int i=0; i<(leng-2); i++) {
      receiveSum = receiveSum + thebuf[i];
    }
    receiveSum = receiveSum + 0x42;

    if (receiveSum == ((thebuf[leng - 2] << 8) + thebuf[leng - 1])) {
      receiveSum = 0;
      receiveflag = 1;
    }
    return receiveflag;
  }

  int transmitPM01(unsigned char* thebuf) {
    int PM01Val;
    PM01Val = ((thebuf[3] << 8) + thebuf[4]);
    return PM01Val;
  }

  int transmitPM2_5(unsigned char* thebuf) {
    int PM2_5Val;
    PM2_5Val = ((thebuf[5] << 8) + thebuf[6]);
    return PM2_5Val;
  }

  int transmitPM10(unsigned char* thebuf) {
    int PM10Val;
    PM10Val = ((thebuf[7] << 8) + thebuf[8]);
    return PM10Val;
  }
};

#endif
