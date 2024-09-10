#include <TinyGPS++.h>

TinyGPSPlus gps;

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, 16, 17);

  Serial.println("Iniciando GPS...");
}

void readDataGps() {
  while (Serial2.available() > 0) {
    char c = Serial2.read();
    gps.encode(c);
    Serial.print(c);
  }
}

void loop() {
  readDataGps();
  delay(500);
}
