#include <Wire.h>
#include <Adafruit_SHT31.h>
#include <SD.h>
#include <FS.h>

#define SD_CS 5

Adafruit_SHT31 sht31 = Adafruit_SHT31();
float sht31_temp, sht31_hum;

void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

void setup() {
  Serial.begin(115200);

  if (!sht31.begin(0x44)) {
    Serial.println("ERROR SHT31");
    while (1);
  }

  if (!SD.begin(SD_CS)) {
    Serial.println("Card Mount Failed");
    return;
  }

  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }
  Serial.println("SD card initialized");

  File file = SD.open("/datalog.txt");
  if (!file) {
    Serial.println("File doesn't exist, creating file...");
    writeFile(SD, "/datalog.txt", "");
  } else {
    Serial.println("File already exists");
  }
  file.close();
}

void loop() {
  sht31_temp = sht31.readTemperature();
  sht31_hum = sht31.readHumidity();

  Serial.print("Temperature: ");
  Serial.print(sht31_temp);
  Serial.println(" *C");
  Serial.print("Humidity: ");
  Serial.print(sht31_hum);
  Serial.println(" %");

  String data = "Temperature: " + String(sht31_temp) + " *C, Humidity: " + String(sht31_hum) + " %\n";
  appendFile(SD, "/datalog.txt", data.c_str());

  delay(1000);
}
