#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>

#define OLED_SDA 21
#define OLED_SCL 22

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

struct SwitchPins {
  int pin1;
  int pin2;
};

// Definimos los pares de pines para cada switch
SwitchPins switchPairs[] = {
  {32, 33},  // Switch 1
  {25, 26},  // Switch 2
  {13, 12},  // Switch 3
  {14, 27}   // Switch 4
};

Adafruit_SH1106 display(OLED_SDA, OLED_SCL);
RF24 radio(4, 5); // (CE, CSN)
const byte address[6] = "00069";

// Definición de pines
const int ky23_x1 = 36;
const int ky23_y1 = 39;
const int ky23_x2 = 34;
const int ky23_y2 = 35;
const int mts103_1 = 32;
const int mts103_2 = 33;
const int mts103_3 = 25;
const int mts103_4 = 26;
const int mts103_5 = 13;
const int mts103_6 = 12;
const int mts103_7 = 14;
const int mts103_8 = 27;
const int pot_1 = 15;
const int pot_2 = 2;
const int encoder_dt = 17;
const int encoder_clk = 16;

volatile int positionEncoded = 0;
volatile int lastEncoded = 0;

void IRAM_ATTR readEncoder() {
  int MSB = digitalRead(encoder_clk);
  int LSB = digitalRead(encoder_dt);
  int encoded = (MSB << 1) | LSB;
  int sum = (lastEncoded << 2) | encoded;

  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) {
    positionEncoded++;
  } else if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) {
    positionEncoded--;
  }
  lastEncoded = encoded;
}

void setup() {
  Serial.begin(115200);  // Aumentado la velocidad del puerto serie

  Wire.begin(OLED_SDA, OLED_SCL);
  display.begin(SH1106_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();

  SPI.begin(18, 19, 23, 5);
  if (!radio.begin()) {
    Serial.println("Radio hardware not responding!");
    while (1) {}
  }

  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);
  radio.stopListening();

  Wire.begin(OLED_SDA, OLED_SCL);
  display.begin(SH1106_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.display();

  // Configuración de pines
  pinMode(encoder_clk, INPUT_PULLUP);
  pinMode(encoder_dt, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(encoder_clk), readEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoder_dt), readEncoder, CHANGE);

  const int switchPins[] = {mts103_1, mts103_2, mts103_3, mts103_4,
                            mts103_5, mts103_6, mts103_7, mts103_8
                           };
  for (int i = 0; i < 8; i++) {
    pinMode(switchPins[i], INPUT);
  }
}

void updateDisplay(const DataPacket& data) {
  display.clearDisplay();
  display.setTextSize(1);

  // Joysticks
  display.setCursor(0, 0);
  display.print("J1:");
  display.print(data.xValue_1);
  display.print(",");
  display.print(data.yValue_1);

  display.setCursor(0, 10);
  display.print("J2:");
  display.print(data.xValue_2);
  display.print(",");
  display.print(data.yValue_2);

  // Potenciómetros
  display.setCursor(0, 20);
  display.print("Pot:");
  display.print(data.pot_1);
  display.print(",");
  display.print(data.pot_2);

  // Encoder
  display.setCursor(0, 30);
  display.print("Enc:");
  display.print(data.encoderPos);

  // Estados de los switches
  display.setCursor(0, 40);
  display.print("SW:");
  for (int i = 0; i < 4; i++) {
    display.print(data.switchStates[i * 2]);
    display.print(data.switchStates[i * 2 + 1]);
    if (i < 3) display.print(" ");
  }

  // Estado de transmisión
  display.setCursor(0, 50);
  display.print("TX: ");
  if (radio.write(&data, sizeof(DataPacket))) {
    display.print("OK");
  } else {
    display.print("FAIL");
  }

  display.display();
}

void loop() {
  static unsigned long lastTransmission = 0;
  const unsigned long TRANSMISSION_INTERVAL = 40;

  if (millis() - lastTransmission >= TRANSMISSION_INTERVAL) {
    DataPacket dataPacket;

    // Lectura de joysticks
    dataPacket.xValue_1 = analogRead(ky23_x1);
    dataPacket.yValue_1 = analogRead(ky23_y1);
    dataPacket.xValue_2 = analogRead(ky23_x2);
    dataPacket.yValue_2 = analogRead(ky23_y2);

    // Lectura de switch
    for (int i = 0; i < 4; i++) {
      bool state1 = digitalRead(switchPairs[i].pin1);
      bool state2 = digitalRead(switchPairs[i].pin2);

      Serial.print("Switch ");
      Serial.print(i + 1);
      Serial.print(": Pin1=");
      Serial.print(state1);
      Serial.print(", Pin2=");
      Serial.println(state2);

      // Guardar directamente en el paquete
      dataPacket.switchStates[i * 2] = state1;
      dataPacket.switchStates[i * 2 + 1] = state2;
    }

    // Lectura de potenciómetros
    dataPacket.pot_1 = analogRead(pot_1);
    dataPacket.pot_2 = analogRead(pot_2);
    dataPacket.encoderPos = positionEncoded;

    updateDisplay(dataPacket);

    lastTransmission = millis();
  }
}
