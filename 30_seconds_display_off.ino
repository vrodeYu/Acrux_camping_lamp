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
int x_r = 0;                       // Поточне значення PWM (без корекції)
unsigned long lastSwitchTime = 0;  // Час (millis) останньої "справжньої" зміни режиму
bool isCodeActive = true;          // Прапорець активності коду
int previousMode = -1;             // Зберігаємо попередній режим (неможливе значення)

// Коефіцієнт корекції
uint8_t crt3_8(uint8_t val) {
  return ((uint32_t)(val + 1) * (val + 1) * val) >> 16;
}

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

  if (currentMillis - lastUpdateTime >= 10) {
    lastUpdateTime = currentMillis;

    // Читаємо поточний режим
    int currentMode = light.getMode();

    // ------------------ Відстеження справжньої зміни режиму ------------------
    // Якщо поточний режим відрізняється від попереднього,
    // вважаємо, що відбулась "справжня" зміна (наприклад, 0 -> 1, 1 -> 2, 2 -> 0 тощо).
    if (currentMode != previousMode) {
      // Оновлюємо час, коли режим змінився
      lastSwitchTime = currentMillis;
      // Вмикаємо код, бо маємо нову активну зміну
      isCodeActive = true;
      // Запам'ятовуємо поточний режим як "попередній" на майбутнє
      previousMode = currentMode;
    }

    // Перевіряємо, чи не спливли 5 секунд від останньої зміни режиму
    if ((currentMillis - lastSwitchTime) >= 30000UL) {
      // Якщо так — "відключаємо" код
      isCodeActive = false;
    }
    // ------------------------------------------------------------------------

    // -------------------- Налаштування x_r залежно від режиму ----------------
    // (Виконуємо завжди, або тільки коли активний код — залежить від вимог)
    // Тут, для прикладу, будемо зчитувати x_r лише якщо код активний:
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
      // Якщо код неактивний — вимикаємо
      x_r = 0;
    }
    // ------------------------------------------------------------------------

    // ------------------ Вивід на дисплей та керування світлодіодами ------------------
    if (isCodeActive) {
      // Рахуємо скоригований PWM
      uint8_t corrected_pwm = crt3_8(x_r);

      // --- Відображення на дисплеї ---
      if (currentMode == 1 || currentMode == 2) {
        // Показуємо % від PWM
        display.displayNumber(pwm_to_percent(corrected_pwm));
      } else {
        // Якщо режим 0 або будь-який інший
        uint8_t processPercent = sensor.readFinalProcessPercent();
        display.displayNumber(processPercent);
      }

      // --- Керування LED ---
      if (currentMode == 1) {
        analogWrite(LED_PIN_X, corrected_pwm);
        digitalWrite(LED_PIN_R, LOW);
      }
      else if (currentMode == 2) {
        analogWrite(LED_PIN_R, corrected_pwm);
        digitalWrite(LED_PIN_X, LOW);
      }
      else {
        // Режим 0
        digitalWrite(LED_PIN_X, LOW);
        digitalWrite(LED_PIN_R, LOW);
      }
    }
    else {
      
    }
    // --------------------------------------------------------------------------------
  }
}
