#include "nRF24L01.h"
#include "RF24.h"
#include <SPI.h>

// Estructura de datos que coincide con el transmisor
struct DataPacket {
  int16_t xValue_1;
  int16_t yValue_1;
  int16_t xValue_2;
  int16_t yValue_2;
  uint8_t switchStates[8];
  int16_t pot_1;
  int16_t pot_2;
  int16_t encoderPos;
};

RF24 radio(9, 10); // CE, CSN
const byte address[6] = "00069";

unsigned long lastMillis_rx = 0;
unsigned long interval_rx = 50;
const int ledPower = 5;
const int ledData = 6;
unsigned long lastMillis_ledPower = 0;
unsigned long interval_ledPower = 1000;

// Función auxiliar para interpretar el estado del MTS-103
String interpretSwitchState(uint8_t state1, uint8_t state2) {
  if (state1 && !state2) return "UP";
  if (!state1 && state2) return "DOWN";
  return "CENTER";
}

void setup() {
  Serial.begin(115200);
  Serial.println("Iniciando receptor...");

  if (!radio.begin()) {
    Serial.println("Error al iniciar radio!");
    while (1) {}
  }

  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);
  radio.startListening();

  pinMode(ledPower, OUTPUT);
  pinMode(ledData, OUTPUT);
}

void receive() {
  if (radio.available()) {
    DataPacket dataPacket;
    radio.read(&dataPacket, sizeof(DataPacket));

    Serial.println("\n_____________________________________________________");

    // Joysticks
    Serial.print("Joystick 1 - X: ");
    Serial.print(dataPacket.xValue_1);
    Serial.print(" / Y: ");
    Serial.println(dataPacket.yValue_1);

    Serial.print("Joystick 2 - X: ");
    Serial.print(dataPacket.xValue_2);
    Serial.print(" / Y: ");
    Serial.println(dataPacket.yValue_2);

    for (int i = 0; i < 4; i++) {
      Serial.print("Switch ");
      Serial.print(i + 1);
      Serial.print(": State1=");
      Serial.print(dataPacket.switchStates[i * 2]);
      Serial.print(", State2=");
      Serial.println(dataPacket.switchStates[i * 2 + 1]);
    }

    // Potenciómetros
    Serial.print("\nPot 1: ");
    Serial.print(dataPacket.pot_1);
    Serial.print(" / Pot 2: ");
    Serial.println(dataPacket.pot_2);

    // Encoder
    Serial.print("Encoder: ");
    Serial.println(dataPacket.encoderPos);

    Serial.println("_____________________________________________________");

    digitalWrite(ledData, !digitalRead(ledData));
  }
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - lastMillis_ledPower >= interval_ledPower) {
    lastMillis_ledPower = currentMillis;
    digitalWrite(ledPower, !digitalRead(ledPower));
  }

  if (currentMillis - lastMillis_rx >= interval_rx) {
    lastMillis_rx = currentMillis;
    receive();
  }
}
