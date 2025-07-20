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
      name.remove(name.length() - 4); // ".TXT" sil
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

  file.println("=== Roket Uçuş Logu Başlatıldı ===");
  file.close();
  Serial.print("✅ Yeni log dosyası: ");
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
bool log_three_values(uint8_t a, uint8_t b, uint8_t c) {
  File file = SD.open(currentLogFile.c_str(), FILE_WRITE);
  if (!file) return false;

  file.print(a); file.print(" ");
  file.print(b); file.print(" ");
  file.println(c);

  file.close();
  return true;
}

#endif // SD_LOGGER_H
