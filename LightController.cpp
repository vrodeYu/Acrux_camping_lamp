#include "LightController.h"

/** Базовий конструктор */
LightController::LightController(int analogPin)
  : _analogPin(analogPin),
    _b1Min(820), _b1Max(900),
    _b2Min(400), _b2Max(500),
    _b3Min(600), _b3Max(800),
    _mode(0), _x(100), _r(100),
    _button1Pressed(false),
    _lastButton1State(false),
    _longPressActive(false),
    _lastDebounceTime(0),
    _pressStartTime(0),
    _lastAdjustmentTime(0)
{
}

/** Розширений конструктор */
LightController::LightController(int analogPin,
                                 int b1Min, int b1Max,
                                 int b2Min, int b2Max,
                                 int b3Min, int b3Max)
  : _analogPin(analogPin),
    _b1Min(b1Min), _b1Max(b1Max),
    _b2Min(b2Min), _b2Max(b2Max),
    _b3Min(b3Min), _b3Max(b3Max),
    _mode(0), _x(100), _r(100),
    _button1Pressed(false),
    _lastButton1State(false),
    _longPressActive(false),
    _lastDebounceTime(0),
    _pressStartTime(0),
    _lastAdjustmentTime(0)
{
}

void LightController::begin(unsigned long /*baudRate*/) {
    // Раніше тут могло бути Serial.begin(baudRate), тепер прибрано.
    // Завантажуємо значення з EEPROM
    loadFromMemory();

    // При старті лишаємо режим 0 (не обнуляючи _x, _r)
    _mode = 0;
}

/**
 * Головна функція оновлення:
 * викликається часто, щоб відстежувати кнопки та змінювати X або R.
 */
void LightController::update() {
    int buttonValue = analogRead(_analogPin);

    bool currentButton1State = (buttonValue >= _b1Min && buttonValue <= _b1Max);

    // Дебаунс
    if (currentButton1State != _lastButton1State) {
        _lastDebounceTime = millis();
    }

    if ((millis() - _lastDebounceTime) > debounceDelay) {
        if (currentButton1State && !_button1Pressed) {
            // Натиснули кнопку 1
            _pressStartTime   = millis();
            _button1Pressed   = true;
            _longPressActive  = false;
        }
        else if (!currentButton1State && _button1Pressed) {
            // Відпустили кнопку 1
            handleButtonPress();
        }
    }

    // Перевірка «довгого натискання» (режим 2)
    if (_button1Pressed && !_longPressActive && (millis() - _pressStartTime) >= LONG_PRESS_TIME) {
        resetValues(2);
        _longPressActive = true;
    }

    _lastButton1State = currentButton1State;

    // Кнопка 2 – збільшити
    if (buttonValue >= _b2Min && buttonValue <= _b2Max) {
        adjustValues(1);
    }
    // Кнопка 3 – зменшити
    else if (buttonValue >= _b3Min && buttonValue <= _b3Max) {
        adjustValues(-1);
    }

    delay(5); 
}

/** Зберігаємо поточні значення (x, r) у EEPROM. */
void LightController::saveToMemory() {
    EEPROM.put(ADDR_X, _x);
    EEPROM.put(ADDR_R, _r);
}

/** Зчитуємо значення (x, r) з EEPROM. */
void LightController::loadFromMemory() {
    EEPROM.get(ADDR_X, _x);
    EEPROM.get(ADDR_R, _r);
}

/**
 * Перемикаємо режим, не обнуляючи _x та _r.
 * Якщо попередній режим був 1 або 2, зберігаємо поточні значення в EEPROM,
 * потім читаємо назад (щоб оновити).
 */
void LightController::resetValues(int newMode) {
    if (_mode == 1 || _mode == 2) {
        saveToMemory();
    }
    _mode = newMode;
    loadFromMemory();
}

/**
 * Обробка короткого натискання кнопки 1:
 * якщо був 2 -> 0;
 * якщо був 0 -> 1;
 * якщо був 1 -> 0.
 */
void LightController::handleButtonPress() {
    unsigned long pressDuration = millis() - _pressStartTime;
    _button1Pressed = false;
    if (!_longPressActive && pressDuration < LONG_PRESS_TIME) {
        resetValues((_mode == 2) ? 0 : (_mode ? 0 : 1));
    }
}

/**
 * Змінює _x або _r залежно від режиму 1 чи 2,
 * з урахуванням затримки, щоб не змінювати надто швидко.
 */
void LightController::adjustValues(int delta) {
    // Проміжок часу, з яким дозволено зміну
    const unsigned long adjustmentInterval = 36; // У мс; змініть, якщо потрібно більше/менше

    if (millis() - _lastAdjustmentTime >= adjustmentInterval) {
        _lastAdjustmentTime = millis();

        if (_mode == 1) {
            _x = max(0, min(255, _x + delta));
        } 
        else if (_mode == 2) {
            _r = max(0, min(255, _r + delta));
        }
        saveToMemory();
    }
}

/** Методи-сеттери для зміни порогів кнопок. */
void LightController::setButton1Thresholds(int minVal, int maxVal) {
    _b1Min = minVal;
    _b1Max = maxVal;
}
void LightController::setButton2Thresholds(int minVal, int maxVal) {
    _b2Min = minVal;
    _b2Max = maxVal;
}
void LightController::setButton3Thresholds(int minVal, int maxVal) {
    _b3Min = minVal;
    _b3Max = maxVal;
}
