#ifndef BUZZER_H
#define BUZZER_H

#include <Arduino.h>

class Buzzer {
private:
  int buzzerPin;
  bool state;

public:
  Buzzer() {
    buzzerPin = 5;
    state = false;
  }

  void begin(int pin = 5) {
    buzzerPin = pin;
    pinMode(buzzerPin, OUTPUT);
    off();
  }

  void on() {
    digitalWrite(buzzerPin, HIGH);
    state = true;
  }

  void off() {
    digitalWrite(buzzerPin, LOW);
    state = false;
  }

  void toggle() {
    state ? off() : on();
  }

  void beep(unsigned int duration = 200) {
    on();
    delay(duration);
    off();
  }

  void alarm(unsigned int beepTime = 200, unsigned int pauseTime = 200, uint8_t repeat = 3) {
    for (uint8_t i = 0; i < repeat; i++) {
      beep(beepTime);
      delay(pauseTime);
    }
  }

  bool isOn() {
    return state;
  }
};

#endif
