#include <TinyGPS++.h>
#include <Wire.h>
#include <Adafruit_SHT31.h>
#include <SD.h>
#include <FS.h>
#define SD_CS 5

tyTinyGPSPlus gps;
float latitude, longitude, spd_kmph, course_degrees;
String lat_str, lon_str;
#ifdef ESP32
HardwareSerial SerialGPS(2);
#endif

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
  SerialGPS.begin(9600);
  Serial.println("Iniciando GPS...");

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

void readDataGps() {
  bool datos_impresos = false;

  while (SerialGPS.available() > 0) {
    if (gps.encode(SerialGPS.read())) {
      if (gps.location.isValid() && !datos_impresos) {
        latitude = gps.location.lat();
        lat_str = String(latitude, 7);
        longitude = gps.location.lng();
        lon_str = String(longitude, 7);
        spd_kmph = gps.speed.kmph();
        Serial.println("lat: " + lat_str);
        Serial.println("lon: " + lon_str);
        Serial.print("speed-kmph: ");
        Serial.println(spd_kmph);

        datos_impresos = true;
      }
    }
  }
}

String obtenerDireccionCardinal(float grados) {
  if (grados >= 337.5 || grados < 22.5) return "Norte";
  if (grados >= 22.5 && grados < 67.5) return "Noreste";
  if (grados >= 67.5 && grados < 112.5) return "Este";
  if (grados >= 112.5 && grados < 157.5) return "Sureste";
  if (grados >= 157.5 && grados < 202.5) return "Sur";
  if (grados >= 202.5 && grados < 247.5) return "Suroeste";
  if (grados >= 247.5 && grados < 292.5) return "Oeste";
  if (grados >= 292.5 && grados < 337.5) return "Noroeste";
  return "Desconocido";
}

void obtenerRumbo() {
  if (gps.course.isValid()) {
    course_degrees = gps.course.deg(); // Obtener el rumbo en grados
    String direccion = obtenerDireccionCardinal(course_degrees);

    Serial.print("Rumbo: ");
    Serial.print( course_degrees);
    Serial.print("*/ ");
    Serial.println(direccion);
  } else {
    Serial.println("Esperando datos de rumbo...");
  }
}

void loop() {
  sht31_temp = sht31.readTemperature();
  sht31_hum = sht31.readHumidity();

  Serial.print("Temp: ");
  Serial.print(sht31_temp);
  Serial.print(" /Hum: ");
  Serial.println(sht31_hum);

  //  String data = "Temperature: " + String(sht31_temp) + " *C, Humidity: " + String(sht31_hum) + " %\n";
  //  appendFile(SD, "/datalog.txt", data.c_str());

  readDataGps();
  obtenerRumbo();
  Serial.println("__________________________");
  delay(1000);
}
