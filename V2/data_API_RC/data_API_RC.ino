#include <TinyGPS++.h>
#include <Wire.h>
#include <Adafruit_SHT31.h>
#include <ArduinoJson.h>
#include <HardwareSerial.h>
#include <PM3006S.h>
#include <WiFi.h>
#include <ESP32Time.h>
#include <ESPAsyncWebServer.h>

PM3006S cubic;

TinyGPSPlus gps;
float latitude, longitude, spd_kmph;
String lat_str, lon_str;
float pm1, pm2_5, pm10, tsp, pq0_3, pq0_5, pq1, pq2_5, pq5, pq10;
int err_cubic;

#ifdef ESP32
HardwareSerial SerialPM3006S(0);
HardwareSerial SerialGPS(2);
#endif

Adafruit_SHT31 sht31 = Adafruit_SHT31();
float sht31_temp, sht31_hum;

//Wi-Fi ESP32
ESP32Time rtc;
const char* ssid = "Familia Hogar";
const char* password = "famapa_men2024##";
const char* ntpServer = "pool.ntp.org";
String utcTimeString;
const long gmtOffset_sec = -4 * 3600;
const int daylightOffset_sec = 0;
AsyncWebServer server(80);

String timeToString(time_t time) {
  struct tm timeInfo;
  gmtime_r(&time, &timeInfo);
  char buffer[20];
  strftime(buffer, sizeof(buffer), "%H:%M:%S", &timeInfo);
  return String(buffer);
}

unsigned long lastProcessTime = 0;
const unsigned long intervalProcess = 4000;

String jsonString;
StaticJsonDocument<512> jsonData;

void setup() {
  Serial.begin(9600);
  cubic.begin(&SerialPM3006S);
  SerialGPS.begin(9600);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());
  server.begin();
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  setupServer();

  if (!sht31.begin(0x44)) {
    Serial.println("ERROR SHT31");
    while (1);
  }
}

void readDataSht31() {
  sht31_temp = sht31.readTemperature();
  sht31_hum = sht31.readHumidity();
}

void readDataGps() {
  while (SerialGPS.available() > 0) {
    if (gps.encode(SerialGPS.read())) {
      if (gps.location.isValid()) {
        latitude = gps.location.lat();
        lat_str = String(latitude, 7);
        longitude = gps.location.lng();
        lon_str = String(longitude, 7);
        spd_kmph = gps.speed.kmph();
      }
    }
  }
}

void readDataPM3006S() {
  String Data = "data";
  cubic.set_read();
  delay(1000);
  err_cubic = cubic.read(&pm1, &pm2_5, &pm10, &tsp, &pq0_3, &pq0_5, &pq1, &pq2_5, &pq5, &pq10);
}

void processDataRead() {
  readDataGps();
  readDataSht31();
  readDataPM3006S();

  jsonData.clear();
  time_t utcTime = rtc.getEpoch();
  utcTimeString = String(timeToString(utcTime));
  String rawDate = rtc.getDate();
  rawDate.replace("," , "-");
  jsonData["date"] = rawDate;
  jsonData["timeUTC"] = utcTimeString;

  if (gps.location.isValid()) {
    jsonData["latitude"] = latitude;
    jsonData["longitude"] = longitude;
    jsonData["speed_kmph"] = spd_kmph;
  } else {
    jsonData["latitude"] = nullptr;
    jsonData["longitude"] = nullptr;
    jsonData["speed_kmph"] = nullptr;
  }

  if (!isnan(sht31_temp) && !isnan(sht31_hum)) {
    jsonData["temperature"] = String(sht31_temp, 2);
    jsonData["humidity"] = String(sht31_hum, 2);
  } else {
    jsonData["temperature"] = nullptr;
    jsonData["humidity"] = nullptr;
  }

  if (!err_cubic) {
    jsonData["pm1"] = pm1;
    jsonData["pm2_5"] = pm2_5;
    jsonData["pm10"] = pm10;
    jsonData["tsp"] = tsp;
  } else {
    jsonData["pm1"] = nullptr;
    jsonData["pm2_5"] = nullptr;
    jsonData["pm10"] = nullptr;
    jsonData["tsp"] = nullptr;
  }

  serializeJson(jsonData, jsonString);
  Serial.println(jsonString);
}

void setupServer() {
  // Ruta para verificar que el servidor está activo
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", "ESP32 API Server is Running");
  });

  // Ruta para obtener los datos en formato JSON
  server.on("/dataRC", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "application/json", jsonString); // Enviar directamente la variable serializada
  });

  server.begin();
}

void loop() {
  if (millis() - lastProcessTime >= intervalProcess) {
    lastProcessTime = millis();
    processDataRead();
  }
}
