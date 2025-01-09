#define SOFTSPI
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <VarSpeedServo.h>
#include <DigitalIO.h>

#define SOFT_SPI_MOSI_PIN A0
#define SOFT_SPI_MISO_PIN A1
#define SOFT_SPI_SCK_PIN A2
#define CE_PIN 2
#define CSN_PIN 4

RF24 radio(CE_PIN, CSN_PIN);
const byte address[6] = "00069";

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

DataPacket data;
VarSpeedServo servos[6];
int current_deg_servos[6];
int servoPins[6] = {3, 5, 11, 10, 9, 6};
int servoSpeed = 20;
int minAngles[6] = {0, 0, 0, 0, 0, 10};
int maxAngles[6] = {179, 179, 179, 179, 179, 179};

void setup() {
  Serial.begin(115200);  // Cambiar a 115200 para coincidir con el TX

  // Configuración de pines SPI
  pinMode(SOFT_SPI_MOSI_PIN, OUTPUT);
  pinMode(SOFT_SPI_MISO_PIN, INPUT);
  pinMode(SOFT_SPI_SCK_PIN, OUTPUT);
  pinMode(CE_PIN, OUTPUT);
  pinMode(CSN_PIN, OUTPUT);

  // Inicialización del radio con la misma configuración que el TX
  if (!radio.begin()) {
    Serial.println(F("Radio hardware not responding!"));
    while (1) {}
  }

  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);
  radio.startListening();

  // Inicialización de servos
  for (int i = 0; i < 6; i++) {
    servos[i].attach(servoPins[i]);
    current_deg_servos[i] = 90;
    servos[i].write(current_deg_servos[i], servoSpeed);
  }
  delay(500);

  Serial.println("Receptor iniciado, esperando datos...");
}

void loop() {
  if (radio.available()) {
    radio.read(&data, sizeof(DataPacket));

    // Debug print
    Serial.println("\n--- Datos Recibidos ---");
    Serial.print("Joy1 X/Y: "); Serial.print(data.xValue_1); Serial.print("/"); Serial.println(data.yValue_1);
    Serial.print("Joy2 X/Y: "); Serial.print(data.xValue_2); Serial.print("/"); Serial.println(data.yValue_2);
    Serial.print("Pots: "); Serial.print(data.pot_1); Serial.print("/"); Serial.println(data.pot_2);
    Serial.print("Switches: ");
    for (int i = 0; i < 8; i++) {
      Serial.print(data.switchStates[i]);
      Serial.print(" ");
    }
    Serial.println();

    int servo1_angle = map(data.xValue_1, 0, 4095, minAngles[0], maxAngles[0]);
    int servo2_angle = map(data.yValue_1, 0, 4095, minAngles[1], maxAngles[1]);
    int servo3_angle = map(data.xValue_2, 0, 4095, minAngles[2], maxAngles[2]);
    int servo4_angle = map(data.yValue_2, 0, 4095, minAngles[3], maxAngles[3]);
    int servo5_angle = map(data.pot_1, 0, 4095, minAngles[4], maxAngles[4]);
    int servo6_angle = map(data.pot_2, 0, 4095, minAngles[5], maxAngles[5]);

    servos[0].write(servo1_angle, servoSpeed);
    servos[1].write(servo2_angle, servoSpeed);
    servos[2].write(servo3_angle, servoSpeed);
    servos[3].write(servo4_angle, servoSpeed);
    servos[4].write(servo5_angle, servoSpeed);
    servos[5].write(servo6_angle, servoSpeed);
  }
  delay(40);
}
