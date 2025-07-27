#ifndef BME280_SENSOR_H
#define BME280_SENSOR_H

#include <Adafruit_BME280.h>
#include <Arduino.h>

class BME280Sensor {
private:
  Adafruit_BME280 bme;
  uint8_t address;
  bool initialized;

public:
  // Constructor
  BME280Sensor(uint8_t adr = 0x77) {
    address = adr;
    initialized = false;
  }

  // Sensörü başlat
  bool begin() {
    if (bme.begin(address)) {
      initialized = true;
      return true;
    } else {
      Serial.println("BME280 bulunamadı. Bağlantıyı kontrol et!");
      return false;
    }
  }

  // Basıncı oku ve döndür (hPa)
  float readPressure() {
    if (!initialized) return NAN;
    return bme.readPressure() / 100.0F;
  }

  // Basıncı ekrana yazdır
  void printPressure() {
    if (!initialized) {
      Serial.println("BME280 başlatılmadı!");
      return;
    }
    float pressure = readPressure();
    Serial.print("Basınç: ");
    Serial.print(pressure);
    Serial.println(" hPa");
  }

  // Sıcaklık oku
  float readTemperature() {
    if (!initialized) return NAN;
    return bme.readTemperature();
  }

  // Nem oku
  float readHumidity() {
    if (!initialized) return NAN;
    return bme.readHumidity();
  }
};

#endif
