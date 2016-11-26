#include <Arduino.h>

#ifndef UTILS_CPP
#define UTILS_CPP

inline void sort(byte a[], byte size) {
  for (byte i=0; i<(size-1); i++) {
    for (byte o=0; o<(size-(i+1)); o++) {
      if (a[o] > a[o+1]) {
        byte t = a[o];
        a[o] = a[o+1];
        a[o+1] = t;
      }
    }
  }
}

inline unsigned long timeMillis() {
  return millis() + 3600000ul;
}

// values have to be > 0
inline byte rollingAverage(byte* arr, byte size, byte newValue) {
  int total = 0;  
  int validCnt = 1;
  for (byte i=0; i<size-1; i++) {
    arr[i] = arr[i+1];  
    if (arr[i] > 0) {
      total += arr[i];
      validCnt++;
    }
  }  
  arr[size-1] = newValue;
  total += newValue;
  return validCnt > 0 ? total / validCnt : 0;
}

#endif
