#include <IRremote.h>

#ifndef INFRARED_CPP
#define INFRARED_CPP

#define IR_CODE_UNKNOWN 0xFFFFFF

class Infrared {

private:
  IRrecv* irrecv;
  decode_results results;

public:
  Infrared(byte irPin) { 
    irrecv = new IRrecv(irPin); 	
  }
  
  void begin() {
    irrecv->enableIRIn();
  }

  unsigned long receive() {
    unsigned long code = IR_CODE_UNKNOWN;

    if (irrecv->decode(&results)) {
      Serial.print("0x");
      Serial.println(results.value, HEX);
      code = results.value;
      delay(250);
      irrecv->resume();
    }

    return code;
  }
};

#endif
