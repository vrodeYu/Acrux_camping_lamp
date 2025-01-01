#include "SW6115.h"

SW6115::SW6115() {}

void SW6115::begin() {
    Wire.begin();
}

uint8_t SW6115::readFinalProcessPercent() {
    return readRegister(REG_FINAL_PERCENT) & 0x7F;  // Маскуємо біти [6:0]
}

uint8_t SW6115::readRegister(uint8_t reg) {
    Wire.beginTransmission(SW6115_ADDR);
    Wire.write(reg);
    Wire.endTransmission();
    Wire.requestFrom(SW6115_ADDR, 1);
    return Wire.read();
}
