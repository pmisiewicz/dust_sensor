#include <PCF8574.h>
#include <Narcoleptic.h>

#include "DustSensor.cpp"
#include "AirQualityControl.cpp"
#include "Fan.cpp"
#include "LCD.cpp"
#include "RGBLed.cpp"
#include "Buzzer.cpp"
#include "TemperatureSensor.cpp"
#include "LightSensor.cpp"

// Digital pins
#define lcdDataPin 2
#define lcdLatchPin 3
#define lcdClockPin 4
#define lcdLightPin 5
#define buzzerPin 6
#define buttonPin 7
#define dhtPin 8
#define fanPwmPin 10
#define relayPowerPin 11
#define relayFanPin 12
#define fanLedPin 13

// Expander pins
#define redPin 0
#define greenPin 1
#define bluePin 2
#define dustSensorSleepPin 3

// Analog pins
#define fotoPin A0
#define brokenPin A1

PCF8574* expander = new PCF8574(); 

TemperatureSensor* temperatureSensor = new TemperatureSensor(dhtPin);
DustSensor* dustSensor = new DustSensor(expander, temperatureSensor, dustSensorSleepPin);
Fan* fan = new Fan(expander, fanPwmPin, fanLedPin, relayPowerPin);
LCD* lcd = new LCD(lcdDataPin, lcdLatchPin, lcdClockPin, lcdLightPin);
RGBLed* rgbLed = new RGBLed(expander, redPin, greenPin, bluePin);
Buzzer* buzzer = new Buzzer(buzzerPin);
LightSensor* lightSensor = new LightSensor(fotoPin);
AirQualityControl* airQualityControl = new AirQualityControl(dustSensor, fan, rgbLed, buzzer, lightSensor);

void setup() {
  Serial.begin(9600);
  Serial.setTimeout(1500);

  expander->begin(0x20);
  lcd->begin(16, 2);
  temperatureSensor->begin();

  pinMode(lcdLightPin, OUTPUT);

  pinMode(fanPwmPin, OUTPUT);
  setPwmFrequency(fanPwmPin, 1);
  analogWrite(fanPwmPin, 0);

  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW); 
  
  pinMode(fanLedPin, OUTPUT);
  digitalWrite(fanLedPin, LOW); 

  pinMode(buttonPin, INPUT);

  pinMode(relayPowerPin, OUTPUT);
  digitalWrite(relayPowerPin, LOW); 

  pinMode(relayFanPin, OUTPUT);
  digitalWrite(relayFanPin, LOW); 

  expander->pinMode(redPin, OUTPUT);
  expander->pinMode(greenPin, OUTPUT);
  expander->pinMode(bluePin, OUTPUT);

  expander->pinMode(dustSensorSleepPin, OUTPUT);
  expander->digitalWrite(dustSensorSleepPin, LOW); 

  startSequence();
}

void startSequence() {
  rgbLed->pink();

  digitalWrite(relayFanPin, HIGH); 

  lcd->setNormalBrightness();
  lcd->setCursor(0, 0);
  lcd->print(F("  OCZYSZCZACZ   ")); 
  lcd->setCursor(0, 1);
  lcd->print(F("   POWIETRZA    ")); 

  fan->changePower(0); 

  for (byte i=1; i<=5; i++) {
    buzzer->sound(500 + i*100, 100);
  }
}

void loop() {  
  lightSensor->update();
    
  //rgbLed->setEnabled(!lightSensor->isNight());
  buzzer->setEnabled(!lightSensor->isNight());   

  dustSensor->update();  
  temperatureSensor->update();
  airQualityControl->update();   
  handleLCD();  

  if (!dustSensor->isSleeping() || fan->getPower() > 0) {
    digitalWrite(relayFanPin, HIGH);   
  } else {
    digitalWrite(relayFanPin, LOW); 
  }

  if (digitalRead(buttonPin) == HIGH) {
    buzzer->sound(1000, 25, 3);
    fan->changePower(255);
    delay(60*1000);
  }

  if (dustSensor->isSleeping() && fan->getPower() == 0) {  
    Narcoleptic.delay(10000);
  } else {
    delay(100);  
  }
}

void serialEvent() {
  dustSensor->onSerialEvent();
}

void handleLCD() {
  if (lightSensor->isNight()) {
    lcd->lightOff();
  } else {
    lcd->setNormalBrightness();
  }
  
  lcd->setCursor(0, 0);
  lcd->print(F("2.5: "));
  lcd->print((int) dustSensor->getPM2_5()); 
  lcd->print(F(" ")); 
  
  lcd->print(F("10: "));
  lcd->print((int) dustSensor->getPM10());
  lcd->print(F("    "));

  lcd->setCursor(0, 1);
  lcd->print(F("T: "));
  lcd->print((int) temperatureSensor->lastReading().t);
  lcd->print(char(223));
  lcd->print(F("C"));

  lcd->print(F(" H: "));
  lcd->print((int) temperatureSensor->lastReading().h); 
  lcd->print(F("%   "));
}
