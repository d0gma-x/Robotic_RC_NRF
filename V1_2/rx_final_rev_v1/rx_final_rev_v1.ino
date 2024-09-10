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

void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);
  radio.startListening();

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(frontLight, OUTPUT);
  pinMode(backLight, OUTPUT);
}

void loop() {
  if (radio.available()) {
    int data[10];
    radio.read(&data, sizeof(data));
    int xValue_1 = data[0];
    int yValue_1 = data[1];
    int buttonState_1 = data[2];
    int xValue_2 = data[3];
    int yValue_2 = data[4];
    int buttonState_2 = data[5];
    int valuePot_1 = data[6];
    int valuePot_2 = data[7];
    int switch_1 = data[8];
    int switch_2 = data[9];
    
    controlMovement(xValue_1, yValue_1);
    lightControl(switch_1, switch_2);
    delay(20);
  }
}

void lightControl (int switch_1, int switch_2) {
  if (switch_1 == 1) {
    digitalWrite(frontLight, LOW);
    } else {
    digitalWrite(frontLight, HIGH);
  }

  if (switch_2 == 1) {
    digitalWrite(backLight, LOW);
  } else {
    digitalWrite(backLight, HIGH);
  }
}

void controlMovement(int xValue_1, int yValue_1) {
  int motorSpeed = map(xValue_1, 0, 1023, -200, 200);

  if (yValue_1 < 341) { // Rango de 0 a 340: girar en el lugar hacia la izquierda
    turnLeftInPlace(200);
  } else if (yValue_1 > 682) { // Rango de 682 a 1023: girar en el lugar hacia la derecha
    turnRightInPlace(200);
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
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);  // Ambos motores detenidos
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}
