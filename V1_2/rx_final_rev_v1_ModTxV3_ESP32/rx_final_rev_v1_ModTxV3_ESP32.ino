#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(9, 10); // CE, CSN
const byte address[6] = "00069";

int ENA = 5;
int IN1 = 2;
int IN2 = 4;
int ENB = 6;
int IN3 = 7;
int IN4 = 8;
int frontLight = A2;
int backLight = A3;
int lastSwitch_1 = 0;
int lastSwitch_2 = 0;
bool frontLightState = false;
bool backLightState = false;

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

DataPacket dataTx;

void setup() {
  Serial.begin(115200);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(frontLight, OUTPUT);
  pinMode(backLight, OUTPUT);

  if (!radio.begin()) {
    Serial.println(F("Radio hardware not responding!"));
    while (1) {}
  }
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);
  radio.startListening();
  Serial.println("Receptor iniciado, esperando datos...");
}

void loop() {
  if (radio.available()) {
    radio.read(&dataTx, sizeof(DataPacket));
    //Debug print
    Serial.println("\n--- Datos Recibidos ---");
    Serial.print("Joy1 X/Y: "); Serial.print(dataTx.xValue_1); Serial.print("/"); Serial.println(dataTx.yValue_1);
    Serial.print("Joy2 X/Y: "); Serial.print(dataTx.xValue_2); Serial.print("/"); Serial.println(dataTx.yValue_2);
    Serial.print("Pots: "); Serial.print(dataTx.pot_1); Serial.print("/"); Serial.println(dataTx.pot_2);
    Serial.print("Switches: ");
    for (int i = 0; i < 8; i++) {
      Serial.print(dataTx.switchStates[i]);
      Serial.print(" ");
    }
    Serial.println();

    controlMovement(dataTx.xValue_1, dataTx.yValue_1);
    lightControl(dataTx.switchStates[1], dataTx.switchStates[3]);
    delay(20);
  }
}

void lightControl(int switch_1, int switch_2) {
  if (switch_1 != lastSwitch_1) {
    frontLightState = !frontLightState;
    digitalWrite(frontLight, frontLightState ? HIGH : LOW);
    lastSwitch_1 = switch_1;
  }

  if (switch_2 != lastSwitch_2) {
    backLightState = !backLightState;
    digitalWrite(backLight, backLightState ? HIGH : LOW);
    lastSwitch_2 = switch_2;
  }
}

//void controlMovement(int16_t xValue_1, int16_t yValue_1) {
//  int motorSpeed = map(xValue_1, 0, 4095, -250, 250); // valor entre 0 y 255
//
//  if (yValue_1 < 1365) { // Rango de 0 a 1365: girar en el lugar hacia la izquierda
//    turnLeftInPlace(220); // valor entre 0 y 255
//  } else if (yValue_1 > 2730) { // Rango de 2730 a 4095: girar en el lugar hacia la derecha
//    turnRightInPlace(220); // valor entre 0 y 255
//  } else {
//    if (motorSpeed > 0) {
//      moveForward(motorSpeed);
//    } else if (motorSpeed < 0) {
//      moveBackward(-motorSpeed);
//    } else {
//      stopMotors();
//    }
//  }
//}

void controlMovement(int16_t xValue_1, int16_t yValue_1) {
  // Definimos una zona muerta
  const int DEADZONE = 100; // Ajusta este valor según sea necesario
  const int CENTRO = 1600;  // Punto medio del rango (4095/2)

  int motorSpeed;

  // Aplicamos la zona muerta al eje X
  if (abs(xValue_1 - CENTRO) < DEADZONE) {
    motorSpeed = 0;
  } else {
    motorSpeed = map(xValue_1, 0, 4095, -255, 255);
  }

  if (yValue_1 < 1365) {
    turnLeftInPlace(220);
  } else if (yValue_1 > 2730) {
    turnRightInPlace(220);
  } else {
    if (motorSpeed > 0) {
      moveForward(motorSpeed);
    } else if (motorSpeed < 0) {
      moveBackward(-motorSpeed);
    } else {
      stopMotors();
    }
  }
}

void moveForward(int speed) {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);   // Motor A con giro horario
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);   // Motor B con giro anti-horario
  analogWrite(ENA, speed);
  analogWrite(ENB, speed);
}

void moveBackward(int speed) {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);  // Motor A con giro anti-horario
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);  // Motor B con giro horario
  analogWrite(ENA, speed);
  analogWrite(ENB, speed);
}

void turnLeftInPlace(int speed) {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);  // Motor A con giro anti-horario
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);   // Motor B con giro horario
  analogWrite(ENA, speed);
  analogWrite(ENB, speed);
}

void turnRightInPlace(int speed) {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);   // Motor A con giro horario
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);  // Motor B con giro anti-horario
  analogWrite(ENA, speed);
  analogWrite(ENB, speed);
}

void stopMotors() {
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);  // Ambos motores detenidos
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  
}
