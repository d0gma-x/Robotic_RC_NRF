#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(9, 10); // CE, CSN
const byte address[6] = "00069";

const int joy_2_x = A0;
const int joy_2_y = A1;
const int joy_1_x = A2;
const int joy_1_y = A3;
const int pot_1 = A4;
const int pot_2 = A5;
const int switchPin_1 = 2;
const int switchPin_2 = 3;
const int switchPin_3 = 7;
const int joy_1_button = 4;
const int joy_2_button = 5;

void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);
  radio.stopListening();

  pinMode(switchPin_1, INPUT_PULLUP);
  pinMode(switchPin_2, INPUT_PULLUP);
  pinMode(switchPin_3, INPUT_PULLUP);
  pinMode(joy_1_button, INPUT_PULLUP);
  pinMode(joy_2_button, INPUT_PULLUP);
}

void loop() {
  int xValue_1 = analogRead(joy_1_x);
  int yValue_1 = analogRead(joy_1_y);
  int buttonState_1 = digitalRead(joy_1_button);

  int xValue_2 = analogRead(joy_2_x);
  int yValue_2 = analogRead(joy_2_y);
  int buttonState_2 = digitalRead(joy_2_button);

  int value_pot_1 = analogRead(pot_1);
  int value_pot_2 = analogRead(pot_2);

  int switchPinState_1 = digitalRead(switchPin_1);
  int switchPinState_2 = digitalRead(switchPin_2);
  int switchPinState_3 = digitalRead(switchPin_3);

  int data[11];
  data[0] = xValue_1;
  data[1] = yValue_1;
  data[2] = buttonState_1;
  data[3] = xValue_2;
  data[4] = yValue_2;
  data[5] = buttonState_2;
  data[6] = value_pot_1;
  data[7] = value_pot_2;
  data[8] = switchPinState_1;
  data[9] = switchPinState_2;
  data[10] = switchPinState_3;

  //  Serial.print("Enviando - JoyX_1: ");
  //  Serial.print(xValue_1);
  //  Serial.print(" /JoyY_1: ");
  //  Serial.print(yValue_1);
  //  Serial.print(" /Button_1: ");
  //  Serial.print(buttonState_1);
  //  Serial.print(" /JoyX_2: ");
  //  Serial.print(xValue_2);
  //  Serial.print(" /JoyY_2: ");
  //  Serial.print(yValue_2);
  //  Serial.print(" /Button_2: ");
  //  Serial.print(buttonState_2);
  //  Serial.print(" /Pot_1: ");
  //  Serial.print(value_pot_1);
  //  Serial.print(" /Pot_2: ");
  //  Serial.print(value_pot_2);
  //  Serial.print(" /Int_1: ");
  //  Serial.print(switchPinState_1);
  //  Serial.print(" /Int_2: ");
  //  Serial.println(switchPinState_2);

  radio.write(&data, sizeof(data));
  delay(40);
}
