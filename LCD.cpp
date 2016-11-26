#include <Arduino.h>
#include <ShiftLCD.h>
#include <LiquidCrystal.h>

#ifndef LCD_CPP
#define LCD_CPP

class LCD {

private:
  ShiftLCD* lcd;
  byte lightPin;
  int brightness = 125;

public:
  LCD(byte lcdDataPin, byte lcdLatchPin, byte lcdClockPin, byte lightPin) { 
    lcd = new ShiftLCD(lcdDataPin, lcdClockPin, lcdLatchPin);
    this->lightPin = lightPin;
  }

  void begin(byte w, byte h) {
    lcd->begin(w, h);
    setNormalBrightness();
  }

  void setCursor(byte x, byte y) {
    lcd->setCursor(x, y);
  }

  void print(String text) {
    lcd->print(text);
  }

  void print(int number) {
    lcd->print(number);
  }

  void print(float number) {
    lcd->print(number);
  }

  void print(char c) {
    lcd->print(c);
  }

  void info(String line1, String line2) {
    lcd->clear();
    lcd->setCursor(0, 0);
    lcd->print(line1);
    lcd->setCursor(0, 1);
    lcd->print(line2);
  }

  void setNormalBrightness() {
    analogWrite(lightPin, brightness);  
  }

  void setLowBrightness() {
    analogWrite(lightPin, 5);  
  }

  void lightOff() {
    analogWrite(lightPin, 0);
  }

  void brighter() {
    brightness += 25;
    if (brightness > 255) {
      brightness = 255;  
    }
    analogWrite(lightPin, brightness); 
  }

  void dimmer() {
    brightness -= 25;
    if (brightness < 0) {
      brightness = 0;  
    }
    analogWrite(lightPin, brightness);  
  }
};

#endif
