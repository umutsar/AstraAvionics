#ifndef GPS_H
#define GPS_H

#include <NMEAGPS.h>  // NeoGPS ana kütüphane
#include "HardwareSerial.h"

NMEAGPS gps;  // GPS ayrıştırıcısı
gps_fix fix;  // Son alınan konum verisi

HardwareSerial& gpsSerial = Serial2;

// -------------------- Başlatıcı --------------------
void init_gps(uint32_t baud = 9600) {
  gpsSerial.begin(baud);
}

// -------------------- GPS Verilerini Güncelle --------------------
void update_gps() {
  while (gpsSerial.available()) {
    if (gps.decode(gpsSerial.read())) {
      fix = gps.read();  // Yeni veriyi al
    }
  }
}

// -------------------- Ham Seri Veriyi Yazdır --------------------
void print_raw_gps_serial() {
  while (gpsSerial.available()) {
    Serial.write(gpsSerial.read());
  }
}

// -------------------- GPS Bilgilerini Yazdır --------------------
void print_gps_data() {
  if (fix.valid.location) {
    Serial.print("Konum: ");
    Serial.print(fix.latitude(), 6);
    Serial.print(", ");
    Serial.println(fix.longitude(), 6);
  } else {
    Serial.println("Konum: Geçersiz");
  }

  if (fix.valid.altitude) {
    Serial.print("Yükseklik: ");
    Serial.print(fix.altitude());
    Serial.println(" m");
  }

  if (fix.valid.speed) {
    Serial.print("Hız: ");
    Serial.print(fix.speed_kph());
    Serial.println(" km/s");
  }

  if (fix.valid.satellites) {
    Serial.print("Uydular: ");
    Serial.println(fix.satellites);
  } else {
    Serial.println("Uydular: Bilgi yok");
  }
}

// -------------------- Getter Fonksiyonları --------------------
bool gps_has_fix() {
  return fix.valid.location;
}

float get_latitude() {
  return fix.latitude();
}

float get_longitude() {
  return fix.longitude();
}

float get_altitude() {
  return fix.altitude();
}

float get_speed_kmph() {
  return fix.speed_kph();
}

uint8_t get_satellite_count() {
  return fix.satellites;
}

#endif
