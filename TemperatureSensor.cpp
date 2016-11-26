#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#include "Utils.cpp"

#ifndef TEMPERATURE_SENSOR_CPP
#define TEMPERATURE_SENSOR_CPP

#define SAMPLES 20

struct TemperatureReading {
  byte t = 0;
  byte h = 0;
};

class TemperatureSensor {

private:
  unsigned long lastReadingTime = 0;
  byte* tarr = new byte[SAMPLES];
  byte* harr = new byte[SAMPLES];

  DHT_Unified* dht;
  TemperatureReading reading;

public:
  TemperatureSensor(byte dhtPin) { 
    dht = new DHT_Unified(dhtPin, DHT11);

    for (byte i=0; i<SAMPLES; i++) {
      tarr[i] = 0;  
      harr[i] = 0;  
    }
  }
  
  void begin() {
    dht->begin();
  }

  void update() {
    if (lastReadingTime > timeMillis() - 1000ul * 5ul) {
      return;
    }
    
    sensors_event_t event;   
       
    dht->temperature().getEvent(&event);
    if (event.temperature > 0) {
      reading.t = (byte) rollingAverage(tarr, SAMPLES, event.temperature);
    }
    
    dht->humidity().getEvent(&event);
    if (event.relative_humidity > 0) {
      reading.h = (byte) rollingAverage(harr, SAMPLES, event.relative_humidity);
    }

    lastReadingTime = timeMillis();
  }

  TemperatureReading lastReading() {
    return reading;
  }
};

#endif
