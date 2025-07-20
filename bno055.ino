#ifndef BNO_055
#define BNO_055

#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include <Arduino.h>

Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);  // I2C adresi 0x28

bool isLaunched = 0;

// ------------------- Başlatıcı -------------------
bool init_bno055() {
  if (!bno.begin()) {
    Serial.println("BNO055 başlatılamadı!");
    return false;
  }

  bno.setExtCrystalUse(true);
  Serial.println("BNO055 başlatıldı.");
  return true;
}

// ------------------- Kalibrasyon Durumu -------------------
void get_calibration(uint8_t* calib_sys, uint8_t* calib_gyro, uint8_t* calib_accel, uint8_t* calib_mag) {
  bno.getCalibration(calib_sys, calib_gyro, calib_accel, calib_mag);
}

// Kalibrasyon tamamlanana kadar bekle
bool wait_for_full_calibration(uint16_t timeout_ms = 15000) {
  uint8_t sys, gyro, accel, mag;
  uint32_t start = millis();
  
  while (millis() - start < timeout_ms) {
    get_calibration(&sys, &gyro, &accel, &mag);
    Serial.print("Kalibrasyon -> SYS:");
    Serial.print(sys);
    Serial.print(" GYRO:");
    Serial.print(gyro);
    Serial.print(" ACC:");
    Serial.print(accel);
    Serial.print(" MAG:");
    Serial.println(mag);

    if (sys == 3 && gyro == 3 && accel == 3 && mag == 3) {
      Serial.println("✅ Tam kalibrasyon tamamlandı!");
      return true;
    }

    delay(500);
  }

  Serial.println("⚠️ Kalibrasyon tamamlanamadı, sensör yine de çalışabilir.");
  return false;
}

// ------------------- Yönelim (Yaw, Pitch, Roll) -------------------
void get_orientation(float* out) {
  sensors_event_t data;
  bno.getEvent(&data);
  out[0] = data.orientation.x;  // Yaw
  out[1] = data.orientation.y;  // Pitch
  out[2] = data.orientation.z;  // Roll
}

void print_orientation() {
  float ori[3];
  get_orientation(ori);
  Serial.print("Yaw: ");
  Serial.print(ori[0]);
  Serial.print(" | Pitch: ");
  Serial.print(ori[1]);
  Serial.print(" | Roll: ");
  Serial.println(ori[2]);
}

// ------------------- Lineer İvme (Kalkış Tespiti) -------------------
void get_linear_accel(float* out) {
  imu::Vector<3> v = bno.getVector(Adafruit_BNO055::VECTOR_LINEARACCEL);
  out[0] = v.x(); out[1] = v.y(); out[2] = v.z();
}

bool detect_liftoff(float threshold = 2.0) {
  float lin[3];
  get_linear_accel(lin);
  float totalAccel = sqrt(lin[0]*lin[0] + lin[1]*lin[1] + lin[2]*lin[2]);
  
  return totalAccel > threshold;
}

#endif
