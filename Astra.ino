#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

#include "BME280.h"
#include "BNO055.h"
#include "SD_CARD.h"
#include "LED_BLINK.h"
#include "GPS.h"
#include "RFD.h"
#include "BUZZER.h"

float dummy_data[3] = { 0 };

String currentFileName;
RFD rfd(Serial4, 57600);
BME280Sensor bme280;
BNO055Sensor bno055;  // BNO için OOP sınıfımız
Buzzer buzzer;

float dataPacket[5] = { 12.34, 56.78, 90.12, 34.56, 78.90 };

void setup() {
  Serial.begin(115200);
  Serial4.begin(57600);
  rfd.begin();
  bme280.begin();

  // ✅ BNO başlat
  if (!bno055.begin(0x28)) {
    Serial.println("BNO sensörü başlatılamadı!");
  } else {
    bno055.waitForFullCalibration(10000);  // Opsiyonel (10 saniye kalibrasyon bekler)
  }

  buzzer.begin(5);
  init_gps(9600);
  pinMode(5, OUTPUT);

  if (init_sd_logger()) {
    create_new_log_file();
  }

  led_setup();
  Serial.println("GPS yapılandırması tamam.");
  delay(100);
}

void loop() {
  update_gps();

  // buzzer.beep(300);  // 300 ms bip
  delay(1000);

  if (gps_has_fix()) {
    print_gps_data();
    toggle_led_external();
  } else {
    toggle_led_internal();
  }

  float dataPacket[14] = { 170, 22, 33, 44, 55, 66, 77, 88, 99, 11, 22, 33, 44, 55 };
  print_raw_gps_serial();

  bme280.printPressure();

  bno055.printOrientation();  // Yaw, Pitch, Roll


  float orientation[3] = { 0 };
  bno055.getOrientation(orientation);
  Serial.print("Yaw: ");
  Serial.print(orientation[0]);
  Serial.print(" | Pitch: ");
  Serial.print(orientation[1]);
  Serial.print(" | Roll: ");
  Serial.println(orientation[2]);

  // ✅ Kalkış tespiti:
  if (bno055.detectLiftoff(3.0)) {
    Serial.println("🚀 Roket Fırlatıldı!");
  }

  log_values_float(dataPacket, sizeof(dataPacket) / sizeof(dataPacket[0]));
  delay(300);
}
