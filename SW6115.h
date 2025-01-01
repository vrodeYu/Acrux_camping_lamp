#ifndef SW6115_H
#define SW6115_H

#include <Wire.h>

#define SW6115_ADDR 0x3C  // Приклад адреси пристрою
#define REG_FINAL_PERCENT 0x7E  // Регістр завершення процесу

class SW6115 {
public:
    SW6115();
    void begin();
    uint8_t readFinalProcessPercent();

private:
    uint8_t readRegister(uint8_t reg);
};

#endif  // SW6115_H
