#ifndef BME_280
#define BME_280

#include <Adafruit_BME280.h>


#define BME280_ADR 0x77

Adafruit_BME280 bme;
  // if (!bme.begin(I2C_ADDRESS)) {
  //   Serial.println("BME280 bulunamadı. Bağlantıyı kontrol et!");
  //   while (1);
  // }

float printPressure(Adafruit_BME280& sensor) {
  float pressure = sensor.readPressure() / 100.0F;  // hPa
  Serial.print("Basınç: ");
  Serial.print(pressure);
  Serial.println(" hPa");
  return pressure;
}

#endif

