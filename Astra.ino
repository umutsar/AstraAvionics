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
#include "Detector.h"

// Sensör sınıfları



uint8_t mainParasut[6] = { 0xAA, 0x80, 0x00, 0x00, 0x0D, 0x0A };
uint8_t specific[6] = { 0xAA, 0x40, 0x00, 0x00, 0x0D, 0x0A };
uint8_t drogueParasut[6] = { 0xAA, 0x20, 0x00, 0x00, 0x0D, 0x0A };
uint8_t descent[6] = { 0xAA, 0x10, 0x00, 0x00, 0x0D, 0x0A };

uint8_t angle[6] = { 0xAA, 0x08, 0x00, 0x00, 0x0D, 0x0A };
uint8_t altitudeTh[6] = { 0xAA, 0x04, 0x00, 0x00, 0x0D, 0x0A };
uint8_t burnout[6] = { 0xAA, 0x02, 0x00, 0x00, 0x0D, 0x0A };
uint8_t launch[6] = { 0xAA, 0x01, 0x00, 0x00, 0x0D, 0x0A };

uint8_t* currentSUTPacket = nullptr;


BME280Sensor bme280;
BNO055Sensor bno055;
Buzzer buzzer;
bool roketHavalandi = 0;
// Test modları
enum TestMode { NONE,
                SIT,
                SUT };
TestMode currentMode = NONE;

// Zamanlama
unsigned long lastSendTime = 0;
const unsigned long sendInterval = 100;  // 10 Hz

// Float → Byte dönüşümü
typedef union {
  float f;
  uint8_t bytes[4];
} FloatConverter;

float sut_altitude = 0;
float sut_pressure = 0;
float sut_acc[3] = { 0 };
float sut_angle[3] = { 0 };

float bytesToFloat(uint8_t* data, int startIndex) {
  FloatConverter fc;
  fc.bytes[3] = data[startIndex];
  fc.bytes[2] = data[startIndex + 1];
  fc.bytes[1] = data[startIndex + 2];
  fc.bytes[0] = data[startIndex + 3];
  return fc.f;
}

// Komut okuma buffer'ı
uint8_t commandBuffer[5];
int commandIndex = 0;

// ----------------- Başlatma -----------------

void setup() {
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  Serial.begin(115200);   // Debug portu
  Serial3.begin(115200);  // RS232 iletişim (Test cihazı)

  bme280.begin();
  bno055.begin(0x28);
  buzzer.begin(5);
  init_gps(9600);
  led_setup();

  if (init_sd_logger()) {
    create_new_log_file();
  }

  Serial.println("✅ UKB Sistemi Başladı");
  delay(100);
}

// ----------------- Yardımcı Fonksiyonlar -----------------

float roundTo2(float x) {
  return round(x * 100.0) / 100.0;
}

void writeFloatToBuffer(uint8_t* buffer, int& index, float value) {
  FloatConverter fc;
  fc.f = roundTo2(value);
  for (int i = 3; i >= 0; i--) {
    buffer[index++] = fc.bytes[i];  // Big Endian
  }
}

void sendPacketToSerial3(uint8_t* data, size_t length) {
  Serial3.write(data, length);
}

// ----------------- SİT Paketi Gönderimi -----------------

void sendSITPacket() {
  float altitude = bme280.getAltitude();
  float pressure = bme280.readPressure();
  float acc[3] = { 0 }, angle[3] = { 0 };
  bno055.getAcceleration(acc);
  bno055.getOrientation(angle);

  uint8_t buffer[36];
  int index = 0;

  buffer[index++] = 0xAB;
  writeFloatToBuffer(buffer, index, altitude);
  writeFloatToBuffer(buffer, index, pressure);
  writeFloatToBuffer(buffer, index, acc[0]);
  writeFloatToBuffer(buffer, index, acc[1]);
  writeFloatToBuffer(buffer, index, acc[2]);
  writeFloatToBuffer(buffer, index, angle[0]);
  writeFloatToBuffer(buffer, index, angle[1]);
  writeFloatToBuffer(buffer, index, angle[2]);

  uint8_t checksum = 0;
  for (int i = 0; i < index; i++) checksum += buffer[i];
  buffer[index++] = checksum % 256;

  buffer[index++] = 0x0D;
  buffer[index++] = 0x0A;

  sendPacketToSerial3(buffer, index);
}

#define PACKET_SIZE 36
uint8_t sut_buffer[PACKET_SIZE] = { 0 };
// ----------------- Komut Dinleme -----------------

void sendFlagsPacket() {
  uint8_t header = 0xAA;
  uint16_t flags = 1 << 4;  // Bit 4 aktif, yani 0x0010

  uint8_t data1 = (flags >> 8) & 0xFF;  // Üst byte (0x00)
  uint8_t data2 = flags & 0xFF;         // Alt byte  (0x10)

  uint8_t checksum = (header + data1 + data2) % 256;

  uint8_t packet[6] = { header, data1, data2, checksum, 0x0D, 0x0A };

  Serial3.write(packet, 6);

  // Debug
  Serial.print("Gönderilen paket: ");
  for (int i = 0; i < 6; i++) {
    Serial.print("0x");
    if (packet[i] < 0x10) Serial.print("0");
    Serial.print(packet[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}
void sendContinuousSUT() {
  static unsigned long lastSUTSendTime = 0;
  const unsigned long sutSendInterval = 100;  // 10 Hz

  if (currentSUTPacket == nullptr) return;

  unsigned long now = millis();
  if (now - lastSUTSendTime >= sutSendInterval) {
    Serial3.write(currentSUTPacket, 6);
    lastSUTSendTime = now;
  }
}

void listenSUT() {
  while (Serial3.available() >= PACKET_SIZE) {
    // Header senkronizasyonu (0xAB arıyoruz)
    if (Serial3.peek() != 0xAB) {
      Serial3.read();  // İlk bayt 0xAB değilse at
      continue;
    }

    // Geçici olarak tüm veriyi bir tamponda oku
    Serial3.readBytes(sut_buffer, PACKET_SIZE);

    // Footer kontrolü
    if (sut_buffer[34] == 0x0D && sut_buffer[35] == 0x0A) {
      // ✅ Geçerli paket

      // // 📥 Okunan Baytları Yazdır
      // Serial.println("📥 Okunan Baytlar:");
      // for (int i = 0; i < PACKET_SIZE; i++) {
      //   Serial.print("0x");
      //   if (sut_buffer[i] < 0x10) Serial.print("0");
      //   Serial.print(sut_buffer[i], HEX);
      //   Serial.print(" ");
      // }
      // Serial.println();

      // 🟡 AçıY kontrolü
      float angleY = bytesToFloat(sut_buffer, 25);

      // Verileri değişkenlere aktar
      sut_altitude = bytesToFloat(sut_buffer, 1);
      sut_pressure = bytesToFloat(sut_buffer, 5);
      sut_acc[0] = bytesToFloat(sut_buffer, 9);
      sut_acc[1] = bytesToFloat(sut_buffer, 13);
      sut_acc[2] = bytesToFloat(sut_buffer, 17);
      sut_angle[0] = bytesToFloat(sut_buffer, 21);
      sut_angle[1] = bytesToFloat(sut_buffer, 25);
      sut_angle[2] = bytesToFloat(sut_buffer, 29);

      // Terminal çıktısı
      // Serial.println("📦 SUT VERİLERİ:");
      Serial.println();

      // Serial.print("İrtifa: ");
      // Serial.println(sut_altitude);
      // Serial.print("Basınç: ");
      // Serial.println(sut_pressure);
      // Serial.print("İvme X: ");
      // Serial.println(sut_acc[0]);
      // Serial.print("İvme Y: ");
      // Serial.println(sut_acc[1]);
      // Serial.print("İvme Z: ");
      // Serial.println(sut_acc[2]);
      // Serial.print("Açı X : ");
      // Serial.println(sut_angle[0]);
      // Serial.print("Açı Y : ");
      // Serial.println(sut_angle[1]);
      // Serial.print("Açı Z : ");
      // Serial.println(sut_angle[2]);

      // 🎯 Zirve tespiti

      if (roketHavalandiMi(sut_acc[2])) {
        currentSUTPacket = launch;
        Serial.println("Mal havalandı");
      }
      if (isBurnedout()) {
        currentSUTPacket = burnout;
        Serial.println("Burned out oldu");
      }

      if (kilitNoktasinaGeldiMi(sut_altitude)) {
        currentSUTPacket = altitudeTh;
        Serial.println("kilit noktasına geldi");
      }

      if (apogeeGeldiMi(sut_altitude)) {
        currentSUTPacket = angle;
        Serial.println("Apogee ye gelindi");
      }

      if (isDescent(sut_altitude)) {
        currentSUTPacket = descent;
        Serial.println("İrtifa azaldı");
      }

      if (isDrogueParachute(sut_altitude)) {
        currentSUTPacket = drogueParasut;
        digitalWrite(3, HIGH);
        Serial.println("Drag paraşütü açıldı");
      }

      if (isSpecificAltitude(sut_altitude)) {
        currentSUTPacket = specific;
        Serial.println("Main paraşütün kilidi açıldı");
      }

      if (isMainParachute(sut_altitude)) {
        currentSUTPacket = mainParasut;
        digitalWrite(3, HIGH);

        Serial.println("Main paraşüt açıldı");
      }



    } else {
      // ❌ Paket geçersizse başlığı bile kayabilir, sıradaki bayta geç
      Serial.println("❌ Geçersiz paket yapısı.");
      // Paket bozuksa buffer'ı kaydırarak tekrar aramaya devam et
      continue;
    }
  }
}

void listenForCommands() {
  while (Serial3.available()) {
    uint8_t incomingByte = Serial3.read();
    if (commandIndex == 0 && incomingByte != 0xAA) continue;

    commandBuffer[commandIndex++] = incomingByte;

    if (commandIndex == 5) {
      uint8_t header = commandBuffer[0];
      uint8_t command = commandBuffer[1];
      uint8_t checksum = commandBuffer[2];
      uint8_t footer1 = commandBuffer[3];
      uint8_t footer2 = commandBuffer[4];
      uint8_t expectedChecksum = (header + command) % 256;

      if (header == 0xAA && checksum == expectedChecksum && footer1 == 0x0D && footer2 == 0x0A) {
        switch (command) {
          case 0x20:
            currentMode = SIT;
            Serial.println("✅ Komut: SİT Başlat");
            break;
          case 0x22:
            currentMode = SUT;
            Serial.println("✅ Komut: SUT Başlat (işlem yapılmıyor)");
            break;
          case 0x24:
            currentMode = NONE;
            Serial.println("⛔ Komut: Test Durduruldu");
            break;
          default:
            Serial.println("❓ Geçerli ama bilinmeyen komut");
            break;
        }
      } else {
        Serial.println("❌ Hatalı komut");
      }
      commandIndex = 0;
    }
  }
}

// ----------------- Ana Döngü -----------------


void loop() {
  if (currentMode != SUT) {
    listenForCommands();
  }


  unsigned long now = millis();
  if (now - lastSendTime >= sendInterval) {
    if (currentMode == SIT) {
      Serial.println("Mallar gönderiliyor.");
      sendSITPacket();
    }
    if (currentMode == SUT) {
      // Serial.println("Sut dinleniyor");
      listenSUT();
      sendContinuousSUT();
    }
    lastSendTime = now;
  }

  toggle_led_external();
}
