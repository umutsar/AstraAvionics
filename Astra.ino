#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

#include "bme280.h"
#include "bno055.h"
#include "sd_card.h"
#include "led_blink.h"


float dummy_data[3] = { 0 };  // Yazılacak veriler

String currentFileName;

void setup() {
  Serial.begin(115200);
  bme.begin(BME280_ADR);
  bno.begin();
  pinMode(5, OUTPUT);
  // Start delay

  bno.setExtCrystalUse(true);

  if (init_sd_logger()) {
    create_new_log_file();
  }

  led_setup();
  delay(100);
}

void loop() {

  uint8_t x = random(0, 255);
  uint8_t y = random(0, 255);
  uint8_t z = random(0, 255);

  log_three_values(x, y, z);
  // digitalWrite(5, 1);
  toggle_led_external();
  printPressure(bme);
  print_orientation();


  if (detect_liftoff(3.0)) {
    // Roket havalandı
    Serial.println("Roket Fırlatıldı. ");
  }

  delay(1000);
}
