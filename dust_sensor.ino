#include <PCF8574.h>

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
#define fanPwmPin 10
#define dhtPin 11
#define irPin 12
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
Fan* fan = new Fan(expander, fanPwmPin, fanLedPin);
LCD* lcd = new LCD(lcdDataPin, lcdLatchPin, lcdClockPin, lcdLightPin);
RGBLed* rgbLed = new RGBLed(expander, redPin, greenPin, bluePin);
Buzzer* buzzer = new Buzzer(buzzerPin);
LightSensor* lightSensor = new LightSensor(fotoPin);
AirQualityControl* airQualityControl = new AirQualityControl(dustSensor, fan, rgbLed, buzzer, lightSensor);

boolean discoMode = false;

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

  expander->pinMode(redPin, OUTPUT);
  expander->pinMode(greenPin, OUTPUT);
  expander->pinMode(bluePin, OUTPUT);

  expander->pinMode(dustSensorSleepPin, OUTPUT);
  expander->digitalWrite(dustSensorSleepPin, LOW); 

  startSequence();
}

void startSequence() {
  rgbLed->pink();

  lcd->setNormalBrightness();
  lcd->setCursor(0, 0);
  lcd->print(F("  OCZYSZCZACZ   ")); 
  lcd->setCursor(0, 1);
  lcd->print(F("   POWIETRZA    ")); 

  fan->setMode(FAN_MODE_OFF); 
  airQualityControl->setAutoMode(true);

  for (byte i=1; i<=5; i++) {
    buzzer->sound(500 + i*100, 100);
  }
}

void loop() {  
  lightSensor->update();
    
  rgbLed->setEnabled(!lightSensor->isNight() || discoMode);
  buzzer->setEnabled(!lightSensor->isNight() || discoMode);   

  dustSensor->update();  
  temperatureSensor->update();
  airQualityControl->update();   
  handleLCD();  

  if (digitalRead(buttonPin) == HIGH) {
    discoMode = !discoMode;  
    delay(250);
  }

  if (discoMode) {
    rgbLed->randomColor();
    buzzer->sound(random(100, 2000), random(5, 50));
  }

  delay(100);
}

void serialEvent() {
  dustSensor->onSerialEvent();
}

void handleLCD() {
  if (lightSensor->isNight()) {
    lcd->setLowBrightness();
  } else {
    lcd->setNormalBrightness();
  }
  
  lcd->setCursor(0, 0);
  lcd->print(F("2.5: "));
  lcd->print(dustSensor->getPM2_5()); 
  lcd->print(F(" ")); 
  
  lcd->print(F("10: "));
  lcd->print(dustSensor->getPM10());
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

/**
 * Divides a given PWM pin frequency by a divisor.
 * 
 * The resulting frequency is equal to the base frequency divided by
 * the given divisor:
 *   - Base frequencies:
 *      o The base frequency for pins 3, 9, 10, and 11 is 31250 Hz.
 *      o The base frequency for pins 5 and 6 is 62500 Hz.
 *   - Divisors:
 *      o The divisors available on pins 5, 6, 9 and 10 are: 1, 8, 64,
 *        256, and 1024.
 *      o The divisors available on pins 3 and 11 are: 1, 8, 32, 64,
 *        128, 256, and 1024.
 * 
 * PWM frequencies are tied together in pairs of pins. If one in a
 * pair is changed, the other is also changed to match:
 *   - Pins 5 and 6 are paired on timer0
 *   - Pins 9 and 10 are paired on timer1
 *   - Pins 3 and 11 are paired on timer2
 * 
 * Note that this function will have side effects on anything else
 * that uses timers:
 *   - Changes on pins 3, 5, 6, or 11 may cause the delay() and
 *     millis() functions to stop working. Other timing-related
 *     functions may also be affected.
 *   - Changes on pins 9 or 10 will cause the Servo library to function
 *     incorrectly.
 * 
 * Thanks to macegr of the Arduino forums for his documentation of the
 * PWM frequency divisors. His post can be viewed at:
 *   http://forum.arduino.cc/index.php?topic=16612#msg121031
 */
void setPwmFrequency(int pin, int divisor) {
  byte mode;
  if(pin == 5 || pin == 6 || pin == 9 || pin == 10) {
    switch(divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 64: mode = 0x03; break;
      case 256: mode = 0x04; break;
      case 1024: mode = 0x05; break;
      default: return;
    }
    if(pin == 5 || pin == 6) {
      TCCR0B = TCCR0B & 0b11111000 | mode;
    } else {
      TCCR1B = TCCR1B & 0b11111000 | mode;
    }
  } else if(pin == 3 || pin == 11) {
    switch(divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 32: mode = 0x03; break;
      case 64: mode = 0x04; break;
      case 128: mode = 0x05; break;
      case 256: mode = 0x06; break;
      case 1024: mode = 0x07; break;
      default: return;
    }
    TCCR2B = TCCR2B & 0b11111000 | mode;
  }
}
