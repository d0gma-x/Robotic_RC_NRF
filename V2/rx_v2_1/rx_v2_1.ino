#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <TinyGPS++.h>
#include <Wire.h>
#include <Adafruit_SHT31.h>
#include <ESPAsyncWebServer.h>

RF24 radio(4, 5); // CE, CSN
const byte address[6] = "00069";
const int pwmPin_1 = 25;
const int pwmPin_2 = 33;

TinyGPSPlus gps;
double latitude, longitude, spd_kmph;
#ifdef ESP32
HardwareSerial SerialGPS(2);
#endif

Adafruit_SHT31 sht31 = Adafruit_SHT31();
float sht31_temp, sht31_hum;

struct DataReception {
  int16_t xValue_1;
  int16_t yValue_1;
  int16_t buttonState_1;
  //  int16_t xValue_2;
  //  int16_t yValue_2;
  //  int16_t buttonState_2;
  //  int16_t value_pot_1;
  //  int16_t value_pot_2;
  //  int16_t switchPinState_1;
  //  int16_t switchPinState_2;
};
DataReception dataReception;

unsigned long lastSensorRead = 0;
unsigned long sensorReadInterval = 1000;

const char* ssid = "Familia Hogar";
const char* password = "famapa_men2024##";
AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);
  SerialGPS.begin(9600);
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);
  radio.startListening();

  ledcSetup(0, 5000, 8);
  ledcAttachPin(pwmPin_1, 0);
//  ledcWrite(0, 0);

  ledcSetup(1, 5000, 8);
  ledcAttachPin(pwmPin_2, 1);
//  ledcWrite(1, 0);

  if (!sht31.begin(0x44)) {
    Serial.println("ERROR SHT31");
    while (1);
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a WiFi...");
  }
  Serial.println("Conectado a la red WiFi");
  Serial.print("Accede a la API en: http://");
  Serial.print(WiFi.localIP());
  Serial.println("/datosRC");

  server.on("/datosRC", HTTP_GET, [](AsyncWebServerRequest * request) {
    readDataGPS_SHT31();

    String response = "{";
    response += "\"temperatura\": " + String(sht31_temp) + ",";
    response += "\"humedad\": " + String(sht31_hum) + ",";
    response += "\"latitud\": " + String(latitude, 7) + ",";
    response += "\"longitud\": " + String(longitude, 7) + ",";
    response += "\"velocidad\": " + String(spd_kmph) + "}";

    request->send(200, "application/json", response);
  });
  server.begin();
}

void loop() {
  if (radio.available()) {
    radio.read(&dataReception, sizeof(dataReception));
    controlMovement(dataReception.xValue_1, dataReception.yValue_1);
  }

  if (millis() - lastSensorRead >= sensorReadInterval) {
    lastSensorRead = millis();
    readDataGPS_SHT31();
  }
  delay(20);
}

void controlMovement(int16_t xValue_1, int16_t yValue_1) {
  int16_t motorSpeed;

  if (xValue_1 > 525) {
    motorSpeed = map(xValue_1, 526, 1023, 0, 255);
  } else if (xValue_1 < 505) {
    motorSpeed = map(xValue_1, 504, 0, 0, 255);
  } else {
    motorSpeed = 0;
  }

  motorSpeed = constrain(motorSpeed, 0, 255);

  ledcWrite(0, motorSpeed);
  ledcWrite(1, motorSpeed);

  Serial.print("motorSpeed: ");
  Serial.println(motorSpeed);
}

void readDataGPS_SHT31() {
  while (SerialGPS.available()) {
    if (gps.encode(SerialGPS.read())) {
      if (gps.location.isValid()) {
        latitude = gps.location.lat();
        longitude = gps.location.lng();
        spd_kmph = gps.speed.kmph();
      }
    }
  }

  sht31_temp = sht31.readTemperature();
  sht31_hum = sht31.readHumidity();
}
