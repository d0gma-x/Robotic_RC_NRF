const int potPin = 34;      // Pin analógico para el potenciómetro
const int pwmPin = 25;      // Pin PWM de salida

void setup() {
  pinMode(potPin, INPUT);
  ledcSetup(0, 5000, 8);  // Canal 0, frecuencia de 5kHz, resolución de 8 bits
  ledcAttachPin(pwmPin, 0); // Asigna el pin PWM al canal 0
}

void loop() {
  int potValue = analogRead(potPin);  // Lee el valor del potenciómetro (0 - 4095)
  int pwmValue = map(potValue, 0, 4095, 0, 255);  // Mapea el valor a rango PWM (0 - 255)
  ledcWrite(0, pwmValue);  // Aplica el valor mapeado al PWM
  delay(10);  // Pequeño retraso para estabilidad
}
