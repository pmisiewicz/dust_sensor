#include <Arduino.h>

#ifndef BUZZER_CPP
#define BUZZER_CPP

class Buzzer {

private:
	byte buzzerPin;
  boolean enabled = true;

public:
  Buzzer(byte buzzerPin) { 
    this->buzzerPin = buzzerPin; 	
  }

  void setEnabled(boolean enabled) {
    this->enabled = enabled;  
  }

  Buzzer* sound(int freq, int durationMs) {
    return sound(freq, durationMs, 1);
  }
  
  Buzzer* sound(int freq, int durationMs, byte repeat) {
    if (!enabled) {
      return this;
    }
       
    for (byte i=0; i<repeat; i++) {
      tone(buzzerPin, freq);
      delay(durationMs);
      noTone(buzzerPin);
      delay(10);
    }

    return this;
  }
};

#endif
