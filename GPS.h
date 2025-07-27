#ifndef GPS_H
#define GPS_H

#include <TinyGPS++.h>
#include "HardwareSerial.h"

TinyGPSPlus gps;
HardwareSerial& gpsSerial = Serial2;

void sendUBXCommand(const char* cmd) {
  Serial2.println(cmd);
  delay(200);
}
// -------------------- Başlatıcı --------------------
void init_gps(uint32_t baud = 9600) {
  gpsSerial.begin(baud);


  // Tüm NMEA mesajlarını kapat
  sendUBXCommand("$PUBX,40,GLL,0,0,0,0,0,0*5C");
  sendUBXCommand("$PUBX,40,GSA,0,0,0,0,0,0*4E");
  sendUBXCommand("$PUBX,40,GSV,0,0,0,0,0,0*59");
  sendUBXCommand("$PUBX,40,VTG,0,0,0,0,0,0*5E");

  // Sadece GGA ve RMC aktif et
  sendUBXCommand("$PUBX,40,GGA,0,1,0,0,0,0*5B");
  sendUBXCommand("$PUBX,40,RMC,0,1,0,0,0,0*46");
}

// -------------------- GPS Verilerini Güncelle --------------------
void update_gps() {
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }
}

void print_raw_gps_serial() {
  while (gpsSerial.available()) {
    char c = gpsSerial.read();
    Serial.write(c);
  }
}
// -------------------- GPS Bilgilerini Yazdır --------------------
void print_gps_data() {
  if (gps.location.isValid()) {
    Serial.print("Konum: ");
    Serial.print(gps.location.lat(), 6);
    Serial.print(", ");
    Serial.println(gps.location.lng(), 6);
  } else {
    Serial.println("Konum: Geçersiz");
  }

  if (gps.altitude.isValid()) {
    Serial.print("Yükseklik: ");
    Serial.print(gps.altitude.meters());
    Serial.println(" m");
  }

  if (gps.speed.isValid()) {
    Serial.print("Hız: ");
    Serial.print(gps.speed.kmph());
    Serial.println(" km/s");
  }

  Serial.print("Uydular: ");
  Serial.println(gps.satellites.value());
}

// -------------------- Değer Getiriciler --------------------
bool gps_has_fix() {
  return gps.location.isValid();
}

float get_latitude() {
  return gps.location.lat();
}

float get_longitude() {
  return gps.location.lng();
}

float get_altitude() {
  return gps.altitude.meters();
}

float get_speed_kmph() {
  return gps.speed.kmph();
}

uint8_t get_satellite_count() {
  return gps.satellites.value();
}

#endif