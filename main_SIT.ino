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

// Sensör sınıfları
BME280Sensor bme280;
BNO055Sensor bno055;
Buzzer buzzer;

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

// Komut okuma buffer'ı
uint8_t commandBuffer[5];
int commandIndex = 0;

// ----------------- Başlatma -----------------

void setup() {
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
  for (int i = 3; i >= 0; i--) { // MSB → LSB (Big Endian)
    buffer[index++] = fc.bytes[i];
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

  buffer[index++] = 0xAB;  // Header

  writeFloatToBuffer(buffer, index, altitude);
  writeFloatToBuffer(buffer, index, pressure);
  writeFloatToBuffer(buffer, index, acc[0]);
  writeFloatToBuffer(buffer, index, acc[1]);
  writeFloatToBuffer(buffer, index, acc[2]);
  writeFloatToBuffer(buffer, index, angle[0]);
  writeFloatToBuffer(buffer, index, angle[1]);
  writeFloatToBuffer(buffer, index, angle[2]);

  uint8_t checksum = 0;
  for (int i = 0; i < index; i++) {
    checksum += buffer[i];
  }
  buffer[index++] = checksum % 256;

  buffer[index++] = 0x0D;  // Footer1
  buffer[index++] = 0x0A;  // Footer2

  sendPacketToSerial3(buffer, index);
}

// ----------------- SUT Durum Paketi Gönderimi -----------------

void sendSUTStatusPacket(uint8_t data1, uint8_t data2) {
  uint8_t buffer[6];
  int index = 0;

  buffer[index++] = 0xAA;
  buffer[index++] = data1;
  buffer[index++] = data2;

  uint8_t checksum = (buffer[0] + buffer[1] + buffer[2]) % 256;
  buffer[index++] = checksum;

  buffer[index++] = 0x0D;
  buffer[index++] = 0x0A;

  sendPacketToSerial3(buffer, index);
}

// ----------------- Komut Dinleme (RS232) -----------------

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
            Serial.println("✅ Komut: SUT Başlat");
            break;
          case 0x24:
            currentMode = NONE;
            Serial.println("⛔ Komut: Test Durduruldu");
            break;
          default: Serial.println("❓ Geçerli ama bilinmeyen komut"); break;
        }
      } else {
        Serial.println("❌ Hatalı komut (Checksum veya footer uyuşmuyor)");
      }
      commandIndex = 0;
    }
  }
}

// ----------------- Ana Döngü -----------------

void loop() {
  listenForCommands();  // RS232 üzerinden komut al

  unsigned long now = millis();
  if (now - lastSendTime >= sendInterval) {
    switch (currentMode) {
      case SIT:
        sendSITPacket();
        break;
      case SUT:
        sendSUTStatusPacket(0b00100001, 0b00000000);  // Örnek durum verisi
        break;
      case NONE:
      default:
        // Veri gönderme yok
        break;
    }
    lastSendTime = now;
  }

  toggle_led_external();  // LED yanıp söner
}
