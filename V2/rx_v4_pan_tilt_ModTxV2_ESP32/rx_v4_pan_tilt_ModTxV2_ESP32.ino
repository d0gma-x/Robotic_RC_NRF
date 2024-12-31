#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <ESP32Servo.h>

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

// Antigua estructura (comentada para referencia)
/*struct DataReception {
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
  };*/

const int SERVO_PAN_PIN = 12;
const int SERVO_TILT_PIN = 13;
Servo servoPan;
Servo servoTilt;
int panPosition = 90;
int tiltPosition = 90;

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

  setupServos();
}

void setupServos() {
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  servoPan.setPeriodHertz(50);
  servoTilt.setPeriodHertz(50);
  servoPan.attach(SERVO_PAN_PIN, 500, 2500);
  servoTilt.attach(SERVO_TILT_PIN, 500, 2500);
  servoPan.write(panPosition);
  servoTilt.write(tiltPosition);
}

void loop() {
  if (radio.available()) {
    radio.read(&data, sizeof(DataPacket));

    // Control de movimiento
    controlMovement(data.xValue_1, data.yValue_1);

    // Control de pan/tilt
    bool switchEnabled = (data.switchStates[0] == 1);
    handlePanTilt(switchEnabled, data.xValue_2, data.yValue_2);

    //    Serial.print("Joy1 X/Y: "); Serial.print(data.xValue_1); Serial.print("/"); Serial.println(data.yValue_1);
    //    Serial.print("Joy2 X/Y: "); Serial.print(data.xValue_2); Serial.print("/"); Serial.println(data.yValue_2);
    //    Serial.print("Pot1/2: "); Serial.print(data.pot_1); Serial.print("/"); Serial.println(data.pot_2);
    //    Serial.print("Encoder: "); Serial.println(data.encoderPos);
    for (int i = 0; i < 4; i++) {
      Serial.print("Switch ");
      Serial.print(i + 1);
      Serial.print(": State1=");
      Serial.print(data.switchStates[i * 2]);
      Serial.print(", State2=");
      Serial.println(data.switchStates[i * 2 + 1]);
    }
  }
  delay(20);
}

void controlMovement(int16_t xValue_1, int16_t yValue_1) {
  int16_t motorSpeed = map(xValue_1, 0, 4095, -255, 255);

  if (yValue_1 < 1365) {
    turnLeftInPlace(190);
  } else if (yValue_1 > 2730) { // Ajustado para ADC de ESP32 (4095*2/3)
    turnRightInPlace(190);
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

void handlePanTilt(bool enabled, uint16_t xValue_2, uint16_t yValue_2) {
  if (!enabled) {
    int panIncrement = map(yValue_2, 0, 4095, -2, 2) * -1;
    int tiltIncrement = map(xValue_2, 0, 4095, -2, 2) * -1;

    if (abs(panIncrement) > 0) {
      panPosition += panIncrement;
      panPosition = constrain(panPosition, 0, 179);
      servoPan.write(panPosition);
    }

    if (abs(tiltIncrement) > 0) {
      tiltPosition += tiltIncrement;
      tiltPosition = constrain(tiltPosition, 0, 179);
      servoTilt.write(tiltPosition);
    }
  }
}

void moveForward(int speed) {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  ledcWrite(pwmChannelA, speed);
  ledcWrite(pwmChannelB, speed);
}

void moveBackward(int speed) {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  ledcWrite(pwmChannelA, speed);
  ledcWrite(pwmChannelB, speed);
}

void turnLeftInPlace(int speed) {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  ledcWrite(pwmChannelA, speed);
  ledcWrite(pwmChannelB, speed);
}

void turnRightInPlace(int speed) {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  ledcWrite(pwmChannelA, speed);
  ledcWrite(pwmChannelB, speed);
}

void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  ledcWrite(pwmChannelA, 0);
  ledcWrite(pwmChannelB, 0);
}
