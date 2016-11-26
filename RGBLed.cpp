#include <Arduino.h>
#include <PCF8574.h>

#ifndef RGB_LED_CPP
#define RGB_LED_CPP

class RGBLed {

private:
  PCF8574* expander;
  byte redPin, greenPin, bluePin;
  boolean enabled = true;

  void setColor(byte red, byte green, byte blue) {
    if (!enabled) {
      red = 0;
      green = 0;
      blue = 0;
    }
    
    expander->digitalWrite(redPin, 255 - red);
    expander->digitalWrite(greenPin, 255 - green);
    expander->digitalWrite(bluePin, 255 - blue);  
  }

public:
  RGBLed(PCF8574* expander, byte redPin, byte greenPin, byte bluePin) { 
    this->expander = expander;
    this->redPin = redPin;
    this->greenPin = greenPin;
	  this->bluePin = bluePin;
  }

  void setEnabled(boolean enabled) {
    this->enabled = enabled;  
  }

  void randomColor() {
    byte r = random(0, 2) == 1 ? 255 : 0;
    byte g = random(0, 2) == 1 ? 255 : 0;
    byte b = random(0, 2) == 1 ? 255 : 0;
    setColor(r, g, b);
  }
  
  void none() {
    setColor(0, 0, 0);  
  }

  void red() {
    setColor(255, 0, 0);  
  }

  void green() {
    setColor(0, 255, 0);  
  }

  void blue() {
    setColor(0, 0, 255);  
  }

  void pink() {
    setColor(255, 0, 255);  
  }

  void yellow() {
    setColor(255, 255, 0);  
  }

  void ocean() {
    setColor(0, 255, 255);  
  }

  void white() {
    setColor(255, 255, 255);  
  }
};

#endif
