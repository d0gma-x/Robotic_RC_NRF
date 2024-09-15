#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <TinyGPS++.h>
#include <Wire.h>
#include <Adafruit_SHT31.h>

RF24 radio(4, 5); // CE, CSN
const byte address[6] = "00069";
const int pwmPin_1 = 25;
const int pwmPin_2 = 26;

TinyGPSPlus gps;
float latitude, longitude, spd_kmph;
String lat_str, lon_str;
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

struct DataSend {
  float latitude;
  float longitude;
  float speed_kmph;
  float temperature;
  float humidity;
};
DataSend dataSend;

unsigned long lastSensorRead = 0;
unsigned long sensorReadInterval = 1000;

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

  ledcSetup(1, 5000, 8);
  ledcAttachPin(pwmPin_2, 1);

  if (!sht31.begin(0x44)) {
    Serial.println("ERROR SHT31");
    while (1);
  }
}

void loop() {
  if (radio.available()) {
    radio.read(&dataReception, sizeof(dataReception));
    controlMovement(dataReception.xValue_1, dataReception.yValue_1);
    delay(20);
  }

  if (millis() - lastSensorRead >= sensorReadInterval) {
    lastSensorRead = millis();
    readAndSendData();
  }
  //    delay(20);
}

void controlMovement(int16_t xValue_1, int16_t yValue_1) {
  int16_t motorSpeed;

  if (xValue_1 > 525) {
    motorSpeed = map(xValue_1, 526, 1023, 0, 255);
    ledcWrite(0, motorSpeed);
    ledcWrite(1, motorSpeed);
  } else if (xValue_1 < 505) {
    motorSpeed = map(xValue_1, 504, 0, 0, 255);
    ledcWrite(0, -motorSpeed);
    ledcWrite(1, -motorSpeed);
  } else {
    motorSpeed = 0;
    ledcWrite(0, motorSpeed);
    ledcWrite(1, motorSpeed);
  }

  Serial.print("motorSpeed: ");
  Serial.println(motorSpeed);
}

void printDataSend() {
  Serial.print("Latitud: ");
  Serial.println(dataSend.latitude, 7); // 7 decimales para más precisión

  Serial.print("Longitud: ");
  Serial.println(dataSend.longitude, 7);

  Serial.print("Velocidad (km/h): ");
  Serial.println(dataSend.speed_kmph);

  Serial.print("Temperatura: ");
  Serial.println(dataSend.temperature);

  Serial.print("Humedad: ");
  Serial.println(dataSend.humidity);
}

void readAndSendData() {
  bool datos_impresos = false;

  while (SerialGPS.available() > 0) {
    if (gps.encode(SerialGPS.read())) {
      if (gps.location.isValid() && !datos_impresos) {
        dataSend.latitude = gps.location.lat();
        dataSend.longitude = gps.location.lng();
        dataSend.speed_kmph = gps.speed.kmph();
        datos_impresos = true;

        //        Serial.println("Latitud: " + String(dataSend.latitude, 7));
        //        Serial.println("Longitud: " + String(dataSend.longitude, 7));
        //        Serial.print("Velocidad (km/h): ");
        //        Serial.println(dataSend.speed_kmph);
      }
    }
  }

  dataSend.temperature = sht31.readTemperature();
  dataSend.humidity = sht31.readHumidity();

  //  Serial.print("Temperatura: ");
  //  Serial.println(dataSend.temperature);
  //  Serial.print("Humedad: ");
  //  Serial.println(dataSend.humidity);

  printDataSend();

  bool envioExitoso = radio.write(&dataSend, sizeof(dataSend));
  if (envioExitoso) {
    Serial.println("Datos enviados correctamente");
  } else {
    Serial.println("Error al enviar los datos");
  }
}
