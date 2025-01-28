#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include <Arduino.h>

class ButtonHandler {
private:
    struct Button {
        uint8_t pin;
        unsigned long debounceDelay;
        unsigned long longPressDuration;
        unsigned long lastDebounceTime;
        unsigned long pressStartTime;
        unsigned long lastShortPressTime;
        bool buttonState;
        bool lastButtonState;
        bool isLongPress;
        bool pendingDoublePress;

        Button() : pin(0), debounceDelay(50), longPressDuration(1000), lastDebounceTime(0), pressStartTime(0), lastShortPressTime(0), buttonState(false), lastButtonState(false), isLongPress(false), pendingDoublePress(false) {}

        Button(uint8_t buttonPin, unsigned long debounceTime, unsigned long longPressTime)
            : pin(buttonPin), debounceDelay(debounceTime), longPressDuration(longPressTime),
              lastDebounceTime(0), pressStartTime(0), lastShortPressTime(0), buttonState(false), lastButtonState(false), isLongPress(false), pendingDoublePress(false) {
            pinMode(pin, INPUT_PULLUP);
        }
    };

    Button* buttons;
    uint8_t buttonCount;
    unsigned long doublePressInterval = 500; // Maximum interval for double press detection

public:
    ButtonHandler(uint8_t* buttonPins, uint8_t count, unsigned long debounceTime = 50, unsigned long longPressTime = 1000)
        : buttonCount(count) {
        buttons = new Button[buttonCount];
        for (uint8_t i = 0; i < buttonCount; i++) {
            buttons[i] = Button(buttonPins[i], debounceTime, longPressTime);
        }
    }

    ~ButtonHandler() {
        delete[] buttons;
    }

    void update(void (*onShortPress)(uint8_t buttonIndex), void (*onLongPress)(uint8_t buttonIndex), void (*onDoublePress)(uint8_t buttonIndex), void (*onReleaseAfterHold)(uint8_t buttonIndex)) {
        unsigned long currentTime = millis();

        for (uint8_t i = 0; i < buttonCount; i++) {
            Button& btn = buttons[i];
            bool currentState = digitalRead(btn.pin) == LOW; // Button pressed when LOW

            // Debounce logic
            if (currentState != btn.lastButtonState) {
                btn.lastDebounceTime = currentTime;
            }

            if ((currentTime - btn.lastDebounceTime) > btn.debounceDelay) {
                if (currentState != btn.buttonState) {
                    btn.buttonState = currentState;

                    if (btn.buttonState) {
                        // Button pressed
                        btn.pressStartTime = currentTime;
                        btn.isLongPress = false;
                    } else {
                        // Button released
                        if (btn.isLongPress) {
                            onReleaseAfterHold(i);
                        } else if ((currentTime - btn.pressStartTime) < btn.longPressDuration) {
                            if ((currentTime - btn.lastShortPressTime) <= doublePressInterval) {
                                btn.pendingDoublePress = false;
                                onDoublePress(i);
                                btn.lastShortPressTime = 0; // Reset double press timer
                            } else {
                                btn.pendingDoublePress = true;
                                btn.lastShortPressTime = currentTime;
                            }
                        }
                    }
                }

                // Check for long press
                if (btn.buttonState && !btn.isLongPress && (currentTime - btn.pressStartTime >= btn.longPressDuration)) {
                    btn.isLongPress = true;
                    btn.pendingDoublePress = false; // Cancel pending double press on long press
                    onLongPress(i);
                }
            }

            // Trigger short press if no double press follows
            if (btn.pendingDoublePress && (currentTime - btn.lastShortPressTime > doublePressInterval)) {
                btn.pendingDoublePress = false;
                onShortPress(i);
            }

            btn.lastButtonState = currentState;
        }
    }
};

#endif
