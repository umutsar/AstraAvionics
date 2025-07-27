#ifndef SD_LOGGER_H
#define SD_LOGGER_H

#include <SD.h>
#include <SPI.h>

String currentLogFile = "";

// -------------------- Başlatıcı --------------------
bool init_sd_logger() {
  if (!SD.begin(BUILTIN_SDCARD)) {
    Serial.println("⚠️ SD kart başlatılamadı!");
    return false;
  }
  Serial.println("✅ SD kart hazır.");
  return true;
}

// -------------------- En Büyük Numaralı Dosyayı Bulma --------------------
int find_next_file_number() {
  int maxNumber = -1;

  File root = SD.open("/");
  while (true) {
    File entry = root.openNextFile();
    if (!entry) break;

    String name = entry.name();
    if (name.endsWith(".TXT")) {
      name.toUpperCase();
      name.remove(name.length() - 4);  // ".TXT" sil
      int num = name.toInt();
      if (num > maxNumber) maxNumber = num;
    }

    entry.close();
  }

  return maxNumber + 1;
}

// -------------------- Dosya Oluşturma --------------------
bool create_new_log_file() {
  int nextNumber = find_next_file_number();
  String fileName = String(nextNumber) + ".TXT";
  currentLogFile = fileName;

  File file = SD.open(fileName.c_str(), FILE_WRITE);
  if (!file) {
    Serial.print("⚠️ Dosya oluşturulamadı: ");
    Serial.println(fileName);
    return false;
  }

  file.println("btye1;byte2;byte3;byte4...");
  file.close();
  Serial.println(fileName);
  return true;
}

// -------------------- Satır Loglama --------------------
bool log_line(const String& line) {
  File file = SD.open(currentLogFile.c_str(), FILE_WRITE);
  if (!file) return false;
  file.println(line);
  file.close();
  return true;
}

// -------------------- 3 Byte Veriyi Satır Olarak Logla --------------------
// Float dizisi için
bool log_values_float(const float* values, size_t size) {
  File file = SD.open(currentLogFile.c_str(), FILE_WRITE);
  if (!file) return false;

  for (size_t i = 0; i < size; ++i) {
    file.print(values[i], 2);  // 2 ondalık basamak
    if (i < size - 1) file.print(" ");
  }

  file.println();  // Satır sonu
  file.close();
  return true;
}

bool log_values(const uint8_t* values, size_t size) {
  File file = SD.open(currentLogFile.c_str(), FILE_WRITE);
  if (!file) return false;

  for (size_t i = 0; i < size; ++i) {
    file.print(values[i]);
    if (i < size - 1) file.print(" ");
  }

  file.println();  // Satır sonu
  file.close();
  return true;
}

#endif  // SD_LOGGER_H
