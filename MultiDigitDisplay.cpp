
#include "MultiDigitDisplay.h"

const uint8_t MultiDigitDisplay::segmentMap[14][2] PROGMEM = {
    {2, 3}, {3, 2}, {4, 3}, {4, 2}, {5, 2}, {5, 3}, {5, 4},  // Upper digit segments
    {1, 2}, {2, 1}, {1, 3}, {3, 1}, {1, 4}, {4, 1}, {5, 1}   // Lower digit segments
};

const uint8_t MultiDigitDisplay::digits[10] PROGMEM = {
    0b1111110, 0b0110000, 0b1101101, 0b1111001, 0b0110011,
    0b1011011, 0b1011111, 0b1110000, 0b1111111, 0b1111011
};

MultiDigitDisplay::MultiDigitDisplay(uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pin4, uint8_t pin5) {
    _pins[0] = pin1;
    _pins[1] = pin2;
    _pins[2] = pin3;
    _pins[3] = pin4;
    _pins[4] = pin5;
}

void MultiDigitDisplay::begin() {
    for (uint8_t i = 0; i < 5; i++) {
        pinMode(_pins[i], OUTPUT);
        digitalWrite(_pins[i], LOW);
    }
}

void MultiDigitDisplay::resetPins() {
    for (uint8_t i = 0; i < 5; i++) {
        pinMode(_pins[i], INPUT);
        digitalWrite(_pins[i], LOW);
    }
}

void MultiDigitDisplay::setPins(uint8_t pinHigh, uint8_t pinLow) {
    pinMode(pinHigh, OUTPUT);
    pinMode(pinLow, OUTPUT);
    digitalWrite(pinHigh, HIGH);
    digitalWrite(pinLow, LOW);
}

void MultiDigitDisplay::displayHundreds() {
    setPins(_pins[2], _pins[3]);  // B1
    setPins(_pins[1], _pins[3]);  // C1
    delayMicroseconds(100);
    resetPins();
}

void MultiDigitDisplay::displayDigit(uint8_t digit, bool upperSegment) {
    uint8_t pattern = pgm_read_byte(&digits[digit]);
    uint8_t offset = upperSegment ? 0 : 7;
    
    for (uint8_t seg = 0; seg < 7; seg++) {
        if (pattern & (1 << (6 - seg))) {
            uint8_t pinHigh = pgm_read_byte(&segmentMap[seg + offset][0]);
            uint8_t pinLow = pgm_read_byte(&segmentMap[seg + offset][1]);
            setPins(_pins[pinHigh-1], _pins[pinLow-1]);
            delayMicroseconds(100);
            resetPins();
        }
    }
}

void MultiDigitDisplay::displayNumber(uint8_t number) {
    
    
    if (number == 100) {
        for (uint8_t repeat = 0; repeat < 20; repeat++) {
            displayHundreds();
            displayDigit(0, true);
            displayDigit(0, false);
        }
    } else {
        uint8_t tens = number / 10;
        uint8_t units = number % 10;
        
        for (uint8_t repeat = 0; repeat < 20; repeat++) {
            if (tens > 0) displayDigit(tens, true);
            displayDigit(units, false);
        }
    }
}
