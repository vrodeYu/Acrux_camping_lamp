#ifndef LIGHT_CONTROLLER_H
#define LIGHT_CONTROLLER_H

#include <Arduino.h>
#include <EEPROM.h>

class LightController {
public:
    static const int ADDR_X = 0;
    static const int ADDR_R = 2;
    
    unsigned long getLastPressTime() const { return _pressStartTime; }

    static const unsigned long debounceDelay   = 50UL;
    static const unsigned long LONG_PRESS_TIME = 800UL;

    // Конструктори
    LightController(int analogPin);
    LightController(int analogPin,
                    int b1Min, int b1Max,
                    int b2Min, int b2Max,
                    int b3Min, int b3Max);

    void begin(unsigned long baudRate = 9600);
    void update();

    // Додаємо гетери:
    int getMode() const { return _mode; }
    int getX()    const { return _x;    }
    int getR()    const { return _r;    }

    // Методи для динамічної зміни порогів
    void setButton1Thresholds(int minVal, int maxVal);
    void setButton2Thresholds(int minVal, int maxVal);
    void setButton3Thresholds(int minVal, int maxVal);

private:
    // Пороги кнопок
    int _b1Min, _b1Max;
    int _b2Min, _b2Max;
    int _b3Min, _b3Max;

    int _analogPin;

    // Основні змінні
    int  _mode;
    int  _x;
    int  _r;

    // Керування кнопкою 1 (довге/коротке натискання)
    bool _button1Pressed;
    bool _lastButton1State;
    bool _longPressActive;

    unsigned long _lastDebounceTime;
    unsigned long _pressStartTime;
    unsigned long _lastAdjustmentTime;

    // Приватні методи
    void saveToMemory();
    void loadFromMemory();
    void resetValues(int newMode);
    void handleButtonPress();
    void adjustValues(int delta);
};

#endif
