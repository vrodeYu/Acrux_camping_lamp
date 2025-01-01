#include "LightController.h"
#include "MultiDigitDisplay.h"
#include "SW6115.h"

// Константи для пінів
const uint8_t LED_PIN_X = 9;
const uint8_t LED_PIN_R = 10;

// Тайм-аут неактивності (30 секунд)
const unsigned long CODE_INACTIVITY_TIMEOUT = 5000UL;

// Об'єкти бібліотек
LightController light(A3);
MultiDigitDisplay display(2, 3, 4, 5, 6);
SW6115 sensor;

// Змінні
int x_r = 0;                       // Поточне значення PWM (без корекції)
unsigned long lastSwitchTime = 0;  // Час останньої "справжньої" зміни режиму
bool isCodeActive = true;          // Прапорець активності коду
int previousMode = -1;             // Зберігаємо попередній режим (неможливе значення)

// Коефіцієнт корекції
uint8_t crt3_8(uint8_t val) {
  return ((uint32_t)(val + 1) * (val + 1) * val) >> 16;
}

// Переводимо значення PWM [0..255] у відсотки [0..100]
int pwm_to_percent(uint8_t val) {
  return (val * 100) / 255;
}

void setup() {
  pinMode(LED_PIN_X, OUTPUT);
  pinMode(LED_PIN_R, OUTPUT);

  sensor.begin();
  display.begin();
  light.begin();
}

void loop() {
  // Оновлюємо стан контролера світла
  light.update();

  // Робимо оновлення раз на 10 мс
  static unsigned long lastUpdateTime = 0;
  unsigned long currentMillis = millis();
  if (currentMillis - lastUpdateTime < 10) {
    return; // ще не час оновлюватися
  }
  lastUpdateTime = currentMillis;

  // 1. Визначаємо поточний режим
  int currentMode = light.getMode();

  // 2. Перевіряємо зміну режиму
  if (currentMode != previousMode) {
    lastSwitchTime = currentMillis;
    isCodeActive   = true;
    previousMode   = currentMode;
  }

  // 3. Перевіряємо тайм-аут (30 с)
  if (currentMillis - lastSwitchTime >= CODE_INACTIVITY_TIMEOUT) {
    isCodeActive = false;
  }

  // 4. Визначаємо потрібне значення x_r
  //    (залежить від режиму; активний код чи ні)
  if (isCodeActive) {
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
  } else {
    x_r = 0;
  }

  // 5. Обчислюємо кориговане значення PWM
  uint8_t corrected_pwm = crt3_8(x_r);

  // 6. Відображаємо дані на дисплеї
  //    Якщо код активний і режим 1 або 2 — показуємо відсотки від PWM
  if (isCodeActive) {//    Інакше (режим 0 або код неактивний) — показуємо процес із сенсора
  if (isCodeActive && (currentMode == 1 || currentMode == 2)) {
    display.displayNumber(pwm_to_percent(corrected_pwm));
  } else {
    display.displayNumber(sensor.readFinalProcessPercent());
  }

  // 7. Керування світлодіодами

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
