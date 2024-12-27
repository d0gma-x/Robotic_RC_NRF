#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(2, 5); // CE en GPIO2, CSN en GPIO5
const byte address[6] = "00069";

int ENA = 4;
int IN1 = 32;
int IN2 = 33;
int ENB = 27;
int IN3 = 25;
int IN4 = 26;

int pwmChannelA = 0;
int pwmChannelB = 1;
int pwmFrequency = 5000;
int pwmResolution = 8;

struct DataReception {
  int16_t xValue_1;
  int16_t yValue_1;
  int16_t buttonState_1;
  int16_t xValue_2;
  int16_t yValue_2;
  int16_t buttonState_2;
  int16_t value_pot_1;
  int16_t value_pot_2;
  int16_t switchPinState_1;
  int16_t switchPinState_2;
};
DataReception data;

void setup() {
  Serial.begin(115200);

  SPI.begin(18, 19, 23, 5); // SCK en D18, MISO en D19, MOSI en D23, CSN en D5
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);
  radio.startListening();

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  ledcSetup(pwmChannelA, pwmFrequency, pwmResolution);
  ledcSetup(pwmChannelB, pwmFrequency, pwmResolution);
  ledcAttachPin(ENA, pwmChannelA);
  ledcAttachPin(ENB, pwmChannelB);
}

void loop() {
  if (radio.available()) {
    radio.read(&data, sizeof(data));

    controlMovement(data.xValue_1, data.yValue_1);
//        Serial.print("xValue: ");
//        Serial.print(data.xValue_1);
//        Serial.print(" /yValue: ");
//        Serial.println(data.yValue_1);
//        Serial.print("pot1: ");
//        Serial.print(data.value_pot_1);
//        Serial.print(" /pot2: ");
//        Serial.println(data.value_pot_2);
  }
  delay(20);
}

void controlMovement(int16_t xValue_1, int16_t yValue_1) {
  int16_t motorSpeed = map(xValue_1, 0, 1023, -255, 255);

  if (yValue_1 < 341) { // Girar a la izquierda en el lugar
    turnLeftInPlace(200);
  } else if (yValue_1 > 682) { // Girar a la derecha en el lugar
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

// Función para avanzar
void moveForward(int speed) {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  ledcWrite(pwmChannelA, speed); // Control de velocidad para motor A
  ledcWrite(pwmChannelB, speed); // Control de velocidad para motor B
}

// Función para retroceder
void moveBackward(int speed) {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  ledcWrite(pwmChannelA, speed); // Control de velocidad para motor A
  ledcWrite(pwmChannelB, speed); // Control de velocidad para motor B
}

// Función para girar a la izquierda en el lugar
void turnLeftInPlace(int speed) {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  ledcWrite(pwmChannelA, speed); // Control de velocidad para motor A
  ledcWrite(pwmChannelB, speed); // Control de velocidad para motor B
}

// Función para girar a la derecha en el lugar
void turnRightInPlace(int speed) {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  ledcWrite(pwmChannelA, speed); // Control de velocidad para motor A
  ledcWrite(pwmChannelB, speed); // Control de velocidad para motor B
}

// Función para detener los motores
void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  ledcWrite(pwmChannelA, 0);
  ledcWrite(pwmChannelB, 0);
}
