#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Servo.h>

RF24 radio(9, 10); // CE, CSN
const byte address[6] = "00069";

Servo servoPan;
Servo servoTilt;

void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);
  radio.startListening();

  servoPan.attach(A0);
  servoTilt.attach(3);

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

    //    controlMovement(xValue_1, yValue_1);
    //    lightControl(switch_1, switch_2);
    if (xValue_1 < 500 || xValue_1 > 523 || yValue_1 < 500 || yValue_1 > 523) {
      controlServos(xValue_1, yValue_1);
    } else {
      stopServos();
    }
    delay(20);
  }
}

void controlServos(int xValue_1, int yValue_1) {
  int panSpeed = map(xValue_1, 0, 1023, 0, 180); // 90 es stop
  int tiltSpeed = map(yValue_1, 0, 1023, 0, 180); // 90 es stop

  servoPan.write(panSpeed);
  servoTilt.write(tiltSpeed);
}

void stopServos() {
  servoPan.write(90); // Detener el servo Pan
  servoTilt.write(90); // Detener el servo Tilt
}
