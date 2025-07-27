#ifndef RFD_H
#define RFD_H

#include <Arduino.h>

class RFD {
private:
  HardwareSerial *rfdSerial;
  uint32_t baudRate;

public:
  RFD(HardwareSerial &serialPort, uint32_t baud = 57600) {
    rfdSerial = &serialPort;
    baudRate = baud;
  }

  void begin() {
    rfdSerial->begin(baudRate);
  }

  void sendByte(uint8_t data) {
    rfdSerial->write(data);
  }

  void sendString(const char *str) {
    rfdSerial->print(str);
  }

  // (ASCII format)
  void sendFloat(float value, uint8_t decimalPlaces = 2) {
    rfdSerial->print(value, decimalPlaces);
  }

  // (binary format)
  void sendArray(const uint8_t *arr, size_t length) {
    rfdSerial->write(arr, length);
  }

  int available() {
    return rfdSerial->available();
  }

  int readByte() {
    if (rfdSerial->available()) {
      return rfdSerial->read();
    }
    return -1;
  }

  void debugPrint() {
    while (rfdSerial->available()) {
      Serial.write(rfdSerial->read());
    }
  }
};

#endif
