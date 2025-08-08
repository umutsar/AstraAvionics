#include "core_pins.h"
#ifndef DETECTOR_H
#define DETECTOR_H

#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

int counter = 0;
float eski_irtifa = 0;
int irtifa_counter = 0;
float apogee = 0;
bool isBurnedOutFlag = 0;
uint32_t timer1 = 0;
bool roketHavalandiMiFlag = 0;
bool kilitNoktasinaGeldiMiFlag = 0;
bool isDescentFlag = 0;
bool isSpecificAltitudeFlag = 0;

bool roketHavalandiMi(float ivme_z) {
  if (ivme_z > 5) {
    if (!roketHavalandiMiFlag) {
      timer1 = millis();
    }
    roketHavalandiMiFlag = 1;
    return 1;  // Roket havalandı
  } else return 0;
}


bool apogeeGeldiMi(float irtifa) {
  if (apogee == 0) {  // Daha önce apogee bulunmadıysa
    if (irtifa < eski_irtifa) {
      irtifa_counter++;
    } else {
      irtifa_counter = 0;    // Artış varsa sayacı sıfırla
      eski_irtifa = irtifa;  // Yeni zirve olabilir
    }

    // 10 kez üst üste irtifa düşerse, apogee'ye gelindi say
    if (irtifa_counter >= 10) {
      apogee = eski_irtifa;
      return true;
    }

    return false;
  }

  // Apogee zaten bulunduysa hep true döndür
  return true;
}

bool isBurnedout() {
  if (millis() - timer1 > 6500 && !isBurnedOutFlag && roketHavalandiMiFlag) {
    isBurnedOutFlag = 1;
    return 1;
  } else return 0;
}

bool kilitNoktasinaGeldiMi(float irtifa) {
  if (irtifa > 1300 && !kilitNoktasinaGeldiMiFlag) {
    kilitNoktasinaGeldiMiFlag = 1;
    return 1;
  } else return 0;
}

bool isDescent(float irtifa) {
  if (apogee - 20 > irtifa) {
    isDescentFlag = 1;
    return 1;
  } else {
    return 0;
  }
}

bool isDrogueParachute(float irtifa) {
  if (apogee - 25 > irtifa) {
    isDescentFlag = 1;
    return 1;
  } else {
    return 0;
  }
}

bool isSpecificAltitude(float irtifa) {
  if (irtifa < 800 && apogee > 0) {
    isSpecificAltitudeFlag = 1;
    return 1;
  } else {
    return 0;
  }
}

bool isMainParachute(float irtifa) {
  if (irtifa < 500 && isSpecificAltitudeFlag) {
    return 1;
  } else {
    return 0;
  }
}

#endif
