#include "nRF24L01.h"
#include "RF24.h"
#include <SPI.h>
//#include <SFE_BMP180.h>
//#include <Wire.h>
//#include <ArduinoJson.h>

//SFE_BMP180 bmp180;
//char status;
//double t, p;
unsigned long lastMillis_bmp180 = 0;
unsigned long interval_bmp180 = 2000;

RF24 radio(9, 10); // CE, CSN
const byte address[6] = "00069";

unsigned long lastMillis_rx = 0;
unsigned long interval_rx = 50;

const int ledPower = 5;
const int ledData = 6;
unsigned long lastMillis_ledPower = 0;
unsigned long interval_ledPower = 1000;

void setup() {
  Serial.begin(9600);
  Serial.println("Hi! AvaxScript...");

  //  if (bmp180.begin())
  //    Serial.println("bmp180 OK!");
  //  else
  //  {
  //    Serial.println("bmp180 ERROR!");
  //    while (1);
  //  }

  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);
  radio.startListening();

  pinMode(ledPower, OUTPUT);
  pinMode(ledData, OUTPUT);
}

//void sensor_bmp180() {
//  DynamicJsonDocument doc(64);
//  char buffer[64];
//  status = bmp180.startTemperature();
//  if (status != 0)
//  {
//    delay(status);
//    status = bmp180.getTemperature(t);
//    if (status != 0)
//    {
//      status = bmp180.startPressure(3);
//      if (status != 0)
//      {
//        delay(status);
//        status = bmp180.getPressure(p, t);
//        if (status != 0)
//        {
//          char t_buffer[8];
//          char p_buffer[8];
//          dtostrf(t, 6, 2, t_buffer);
//          dtostrf(p, 6, 2, p_buffer);
//          doc["t_bmp"] = t_buffer;
//          doc["p_bmp"] = p_buffer;
//          serializeJson(doc, buffer);
//          Serial.println(buffer);
//        }
//      }
//    }
//  }
//}

void receive() {
  //  Serial.println("void receiver() OK!");
  if (radio.available()) {
    int data[10];
    radio.read(&data, sizeof(data));
    int xValue_1 = data[0];
    int yValue_1 = data[1];
    int buttonState_1 = data[2];
    int xValue_2 = data[3];
    int yValue_2 = data[4];
    int buttonState_2 = data[5];
    int value_pot_1 = data[6];
    int value_pot_2 = data[7];
    int switchPinState_1 = data[8];
    int switchPinState_2 = data[9];

    // Mostrar datos recibidos en el monitor serie
    Serial.println("_____________________________________________________");
    Serial.print("Joystick 1 - X: ");
    Serial.print(xValue_1);
    Serial.print(" / Y: ");
    Serial.print(yValue_1);
    Serial.print(" / Button_1: ");
    Serial.println(buttonState_1);

    Serial.print("Joystick 2 - X: ");
    Serial.print(xValue_2);
    Serial.print(" / Y: ");
    Serial.print(yValue_2);
    Serial.print(" / Button_2: ");
    Serial.println(buttonState_2);

    Serial.print("Pot 1: ");
    Serial.print(value_pot_1);
    Serial.print(" / Pot 2: ");
    Serial.println(value_pot_2);

    Serial.print("Switch-1: ");
    Serial.print(switchPinState_1);
    Serial.print(" / Switch-2: ");
    Serial.println(switchPinState_2);
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

  //  if (currentMillis - lastMillis_bmp180 >= interval_bmp180) {
  //    lastMillis_bmp180 = currentMillis;
  //    sensor_bmp180();
  //  }

  if (currentMillis - lastMillis_rx >= interval_rx) {
    lastMillis_rx = currentMillis;
    receive();
  }
  delay(200);
}
