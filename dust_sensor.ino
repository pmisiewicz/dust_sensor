#include <PCF8574.h>

#include "DustSensor.cpp"
#include "AirQualityControl.cpp"
#include "Fan.cpp"
#include "LCD.cpp"
#include "RGBLed.cpp"
#include "Infrared.cpp"
#include "Buzzer.cpp"
#include "TemperatureSensor.cpp"
#include "RelayModule.cpp"
#include "LightSensor.cpp"

// Digital pins
#define lcdDataPin 2
#define lcdLatchPin 3
#define lcdClockPin 4
#define buzzerPin 5
#define lcdLightPin 6
#define dhtPin 7
#define irPin 8
#define relay1Pin 9
#define relay2Pin 10
#define relay3Pin 11
#define relay4Pin 12
#define autoModeLedPin 13

// Expander pins
#define redPin 0
#define greenPin 1
#define bluePin 2
#define dustSensorSleepPin 3

// Analog pins
#define fotoPin A0
#define brokenPin A1

PCF8574* expander = new PCF8574(); 

RelayModule* relays = new RelayModule(expander, relay1Pin, relay2Pin, relay3Pin, relay4Pin);
TemperatureSensor* temperatureSensor = new TemperatureSensor(dhtPin);
DustSensor* dustSensor = new DustSensor(expander, temperatureSensor, dustSensorSleepPin);
Fan* fan = new Fan(relays, expander);
LCD* lcd = new LCD(lcdDataPin, lcdLatchPin, lcdClockPin, lcdLightPin);
Infrared* infrared = new Infrared(irPin);
RGBLed* rgbLed = new RGBLed(expander, redPin, greenPin, bluePin);
Buzzer* buzzer = new Buzzer(buzzerPin);
LightSensor* lightSensor = new LightSensor(fotoPin);
AirQualityControl* airQualityControl = new AirQualityControl(dustSensor, fan, rgbLed, buzzer, lightSensor, autoModeLedPin);

boolean deviceOff = false;
boolean nightMode = false;
boolean discoMode = false;

void setup() {
  Serial.begin(9600);
  Serial.setTimeout(1500);

  expander->begin(0x20);
  lcd->begin(16, 2);
  infrared->begin(); 
  temperatureSensor->begin();

  pinMode(lcdLightPin, OUTPUT);

  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW); 
  
  pinMode(relay1Pin, OUTPUT);
  pinMode(relay2Pin, OUTPUT);
  pinMode(relay3Pin, OUTPUT);
  pinMode(relay4Pin, OUTPUT);

  digitalWrite(relay1Pin, LOW); 
  digitalWrite(relay2Pin, LOW); 
  digitalWrite(relay3Pin, LOW); 
  digitalWrite(relay4Pin, LOW); 
  
  pinMode(autoModeLedPin, OUTPUT);
  digitalWrite(autoModeLedPin, LOW); 

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
  if (!deviceOff) {
    lightSensor->update();
    
    rgbLed->setEnabled(!nightMode && !lightSensor->isNight() || discoMode);
    buzzer->setEnabled(!nightMode && !lightSensor->isNight() || discoMode);   

    dustSensor->update();  
    temperatureSensor->update();
    airQualityControl->update();   
    handleLCD();  
  }

  if (discoMode) {
    rgbLed->randomColor();
    buzzer->sound(random(100, 2000), random(5, 50));
  }

  handleIR(); 

  if (!discoMode){
    delay(100);  
  }
}

void serialEvent() {
  dustSensor->onSerialEvent();
}

void handleLCD() {
  if (lightSensor->isNight() || nightMode) {
    lcd->setLowBrightness();
  } else {
    lcd->setNormalBrightness();
  }
  
  if (nightMode) {
    lcd->setCursor(0, 0);
    lcd->print(F("  zzzZZZzzz...  "));
  } else {
    lcd->setCursor(0, 0);
    lcd->print(F("2.5: "));
    lcd->print(dustSensor->getPM2_5()); 
    lcd->print(F(" ")); 
    
    lcd->print(F("10: "));
    lcd->print(dustSensor->getPM10());
    lcd->print(F("    "));
  }

  lcd->setCursor(0, 1);
  lcd->print(F("T: "));
  lcd->print((int) temperatureSensor->lastReading().t);
  lcd->print(char(223));
  lcd->print(F("C"));

  lcd->print(F(" H: "));
  lcd->print((int) temperatureSensor->lastReading().h); 
  lcd->print(F("%   "));
}

void handleIR() {
  unsigned long code = infrared->receive();

  if (code == IR_CODE_UNKNOWN) {
    return;
  }
  
  if (code == 0xC || code == 0x80C) { // OFF
    switchOnOff();
  } else if (deviceOff) {
    // ignore other commands if device is off
  } else if (code == 0x0 || code == 0x800) {
    switchFanMode(0);
  } else if (code == 0x1 || code == 0x801) {
    switchFanMode(1);
  } else if (code == 0x2 || code == 0x802) {
    switchFanMode(2);
  } else if (code == 0x3 || code == 0x803) {
    switchFanMode(3);
  } else if (code == 0x20 || code == 0x820 || code == 0xE5139CA7 || code == 0xAC516266 || code == 0x9912B99A || code == 0x9FA96F) { // P+, UP, RIGHT
    switchFanModeUp();  
  } else if (code == 0x21 || code == 0x821 || code == 0xE4139B12 || code == 0xAD5163FB || code == 0xBE15326E || code == 0xDB9D3097) { // P-, DOWN, LEFT
    switchFanModeDown();
  } else if (code == 0xB411F6DE || code == 0xDD53082F) { // OK
    switchAutoMode();
  } else if (code == 0xD || code == 0x80D) { // MUTE
    switchNightMode();
  } else if (code == 0x38 || code == 0x838) { // SOURCE
    switchSamplingMode();
  } else if (code == 0x10 || code == 0x810) { // VOL+
    lcdBrighter();
  } else if (code == 0x11 || code == 0x811) { // VOL-
    lcdDimmer();
  } else if (code == 0x24 || code == 0x824) { // Incr.surr
    discoMode = !discoMode;
  } else if (code == 0xF280BC5B || code == 0xAD8C62D2) { // FORMAT
    switchCorrection();
  }
}

void switchOnOff() {
  if (!deviceOff) {
    deviceOff = true;
    nightMode = false;
    lcd->info(F("Wylaczanie..."), "");
    buzzer->sound(100, 100, 2);
    fan->setMode(FAN_MODE_OFF);
    dustSensor->sleep();
    lcd->lightOff();
    lcd->info("", "");
    rgbLed->none();
    airQualityControl->setAutoMode(false);
  } else {
    deviceOff = false;
    airQualityControl->setAutoMode(true);
    lcd->setNormalBrightness();
    startSequence();
    dustSensor->reset();
    dustSensor->wake();
  }  
}

void switchFanMode(byte mode) {
  airQualityControl->setAutoMode(false);

  if (mode == FAN_MODE_OFF) {
    lcd->info(F("Moc wentylatora:"), F("   >>> 0% <<<"));
    buzzer->sound(500, 75, 1); 
  } else if (mode == FAN_MODE_1) {
    lcd->info(F("Moc wentylatora:"), F("  >>> 50% <<<"));
    buzzer->sound(1000, 75, 1); 
  } else if (mode == FAN_MODE_2) {
    lcd->info(F("Moc wentylatora:"), F("  >>> 75% <<<"));
    buzzer->sound(1000, 75, 2); 
  } else if (mode == FAN_MODE_3) {
    lcd->info(F("Moc wentylatora:"), F("  >>> 100% <<<"));
    buzzer->sound(1000, 75, 3); 
  }
  
  fan->setMode(mode);
}

void switchFanModeUp() {
  if (fan->getMode() == FAN_MODE_3) {
    buzzer->sound(250, 50, 2);
  } else {
    switchFanMode(fan->getMode() + 1);
  }
}

void switchFanModeDown() { 
  if (fan->getMode() == FAN_MODE_OFF) {
    buzzer->sound(250, 50, 2);
  } else {
    switchFanMode(fan->getMode() - 1);  
  }
}

void switchAutoMode() {
  if (!nightMode) {
    airQualityControl->setAutoMode(true);
    lcd->info(F("Moc wentylatora:"), F("  >>> AUTO <<<"));
    buzzer->sound(2000, 50)->sound(1000, 50)->sound(2000, 50);
  }
}

void switchNightMode() {
  if (!nightMode) {
    lcd->info(F("Tryb:"), F(" >>> NOCNY <<<"));
    buzzer->sound(500, 100)->sound(400, 100)->sound(300, 100);
    airQualityControl->setAutoMode(false);
    dustSensor->sleep();
    fan->setMode(FAN_MODE_1);
    nightMode = true;
  } else {
    nightMode = false;
    lcd->setNormalBrightness();
    lcd->info(F("Tryb:"), F(" >>> DZIENNY <<<"));
    buzzer->setEnabled(!lightSensor->isNight());
    buzzer->sound(300, 100)->sound(400, 100)->sound(500, 100);
    airQualityControl->setAutoMode(true);
    dustSensor->reset();
    dustSensor->wake();
  }
}

void switchSamplingMode() {
  if (nightMode) {
    return;
  }
  
  dustSensor->nextSamplingMode();

  switch (dustSensor->getSamplingMode()) {
    case SAMPLING_MODE_CONTINUOUS:
      lcd->info(F("Probkowanie:"), F(" >>> CIAGLE <<<"));
      buzzer->sound(2000, 75, 1);
      break;
    case SAMPLING_MODE_AVG:
      lcd->info(F("Probkowanie:"), F(">>> SREDNIA <<<"));
      buzzer->sound(2000, 75, 2);
      break;
    case SAMPLING_MODE_POWERSAVE:
      lcd->info(F("Probkowanie:"), F(">>> CO 5 MIN <<<"));
      buzzer->sound(2000, 75, 3);
      break;  
  } 
  
  delay(250);
}

void lcdBrighter() {
  if (!nightMode) {
    lcd->brighter();
    buzzer->sound(500, 50)->sound(3000, 50);
  }
}

void lcdDimmer() {
  if (!nightMode) {
    lcd->dimmer();
    buzzer->sound(3000, 50)->sound(500, 50);
  }
}

void switchCorrection() {
  if (dustSensor->isCorrection()) {
    lcd->info(F("Korekcja:"), F("   >>> NIE <<<"));
    buzzer->sound(800, 50, 2); 
    dustSensor->setCorrection(false); 
    delay(500);  
  } else {
    lcd->info(F("Korekcja:"), F("   >>> TAK <<<"));
    buzzer->sound(1800, 50, 2); 
    dustSensor->setCorrection(true);
    delay(500);
  }
}

