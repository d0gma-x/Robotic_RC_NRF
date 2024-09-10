#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(4, 5); // CE, CSN
const byte address[6] = "00069";
const int pwmPin_1 = 25;
const int pwmPin_2 = 26;

struct DataPackage {
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

DataPackage data;

void setup() {
  Serial.begin(115200);
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);
  radio.startListening();

  ledcSetup(0, 5000, 8);
  ledcAttachPin(pwmPin_1, 0);

  ledcSetup(1, 5000, 8);
  ledcAttachPin(pwmPin_2, 1);

  pinMode(
}

void loop() {
  if (radio.available()) {
    radio.read(&data, sizeof(data));

    controlMovement(data.xValue_1, data.yValue_1);

    //    Serial.println("_____________________________________________________");
    //    Serial.print("Joystick 1 - X: ");
    //    Serial.print(data.xValue_1);
    //    Serial.print(" / Y: ");
    //    Serial.print(data.yValue_1);
    //    Serial.print(" / Button_1: ");
    //    Serial.println(data.buttonState_1);

    //    Serial.print("Joystick 2 - X: ");
    //    Serial.print(data.xValue_2);
    //    Serial.print(" / Y: ");
    //    Serial.print(data.yValue_2);
    //    Serial.print(" / Button_2: ");
    //    Serial.println(data.buttonState_2);
    //
    //    Serial.print("Pot 1: ");
    //    Serial.print(data.value_pot_1);
    //    Serial.print(" / Pot 2: ");
    //    Serial.println(data.value_pot_2);
    //
    //    Serial.print("Switch-1: ");
    //    Serial.print(data.switchPinState_1);
    //    Serial.print(" / Switch-2: ");
    //    Serial.println(data.switchPinState_2);
    //    Serial.println("_____________________________________________________");
  }
  delay(20);
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
