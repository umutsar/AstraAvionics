#include "core_pins.h"
#ifndef LED_BLINK_H
#define LED_BLINK_H

#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

bool isOpenLed = 0;

void led_setup() {
  pinMode(13, OUTPUT);
  pinMode(9, OUTPUT);
}

void toggle_led_internal() {
  if (!isOpenLed) {
    digitalWrite(13, HIGH);
    isOpenLed = 1;
  }

  else {
    digitalWrite(13, LOW);
    isOpenLed = 0;
  }
}
void toggle_led_external() {
  if (!isOpenLed) {
    digitalWrite(9, HIGH);
    isOpenLed = 1;
  }

  else {
    digitalWrite(9, LOW);
    isOpenLed = 0;
  }
}
#endif
