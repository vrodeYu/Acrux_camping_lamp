#ifndef MULTI_DIGIT_DISPLAY_H
#define MULTI_DIGIT_DISPLAY_H

#include <Arduino.h>

class MultiDigitDisplay {
  public:
    MultiDigitDisplay(uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pin4, uint8_t pin5);
    void begin();
    void displayNumber(uint8_t number);
    
  private:
    uint8_t _pins[5];
    static const uint8_t segmentMap[14][2] PROGMEM;
    static const uint8_t digits[10] PROGMEM;
    
    void resetPins();
    void setPins(uint8_t pinHigh, uint8_t pinLow);
    void displayHundreds();
    void displayDigit(uint8_t digit, bool upperSegment);
};

#endif
