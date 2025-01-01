#include "LightController.h"
#include "MultiDigitDisplay.h"
#include "SW6115.h"

// Константи для пінів
const uint8_t LED_PIN_X = 9;
const uint8_t LED_PIN_R = 10;

// Об'єкти бібліотек
LightController light(A3);
MultiDigitDisplay display(2, 3, 4, 5, 6);
SW6115 sensor;

// Змінні
int x_r = 0;

// Функції
uint8_t crt3_8(uint8_t val) {
  // Оптимізована версія без приведення типів
  return ((uint32_t)(val + 1) * (val + 1) * val) >> 16;
}

int pwm_to_percent(uint8_t val) {
  return (val * 100) / 255;
}

void setup() {
  // Налаштування пінів
  pinMode(LED_PIN_X, OUTPUT);
  pinMode(LED_PIN_R, OUTPUT);
  
  // Ініціалізація бібліотек
  sensor.begin();
  display.begin();
  light.begin();
}

void loop() {
  // Оновлення стану контролера
  light.update();

  // Таймер для оновлення значень раз на 10 мс
  static unsigned long lastUpdateTime = 0;
  static bool screenOn = true; // Чи ввімкнений екран
  unsigned long currentMillis = millis();
  
  // Перевірка, чи екран активний
  if (currentMillis - light.getLastPressTime() > 60000 && screenOn) {
    // Вимикаємо екран, якщо кнопки не натискали 1 хвилину
    screenOn = false;
    display.resetPins(); // Вимкнення дисплея
    digitalWrite(LED_PIN_X, LOW);
    digitalWrite(LED_PIN_R, LOW);
  }

  if (currentMillis - lastUpdateTime >= 10 && screenOn) {
    lastUpdateTime = currentMillis;
    
    // Отримання поточного режиму один раз
    int currentMode = light.getMode();
    
    switch (currentMode) {
      case 1:
        x_r = light.getX();
        break;
      case 2:
        x_r = light.getR();
        break;
      default:
        x_r = 0;
        break;
    }

    // Корекція PWM
    uint8_t corrected_pwm = crt3_8(x_r);
    
    // Відображення значення на дисплеї
    if (currentMode != 0) {
      display.displayNumber(pwm_to_percent(corrected_pwm));
    } else {
      uint8_t processPercent = sensor.readFinalProcessPercent();
      display.displayNumber(processPercent);
    }

    // Управління LED
    if (currentMode == 1) {
      analogWrite(LED_PIN_X, corrected_pwm);
      digitalWrite(LED_PIN_R, LOW);
    } 
    else if (currentMode == 2) {
      analogWrite(LED_PIN_R, corrected_pwm);
      digitalWrite(LED_PIN_X, LOW);
    } 
    else {
      digitalWrite(LED_PIN_X, LOW);
      digitalWrite(LED_PIN_R, LOW);
    }
  }
}
