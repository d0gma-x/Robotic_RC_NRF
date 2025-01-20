#include <WiFi.h>
#include <HTTPClient.h>

// Configura los datos de tu red WiFi
const char* ssid = "Familia Hogar";
const char* password = "famapa_men2024##";

// URL de la API alojada en el otro ESP32
const char* apiUrl = "http://192.168.100.29/dataRC";

void setup() {
  // Inicia la comunicación serie
  Serial.begin(115200);
  
  // Conéctate a la red WiFi
  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Conectado a WiFi.");
}

void loop() {
  // Verifica si está conectado a WiFi
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Especifica la URL de la API
    http.begin(apiUrl);

    // Realiza la solicitud GET
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      // Obtén la respuesta de la API
      String payload = http.getString();
      Serial.println("Respuesta de la API:");
      Serial.println(payload);
      
      // Aquí puedes procesar el JSON recibido según sea necesario
    } else {
      Serial.print("Error en la solicitud HTTP: ");
      Serial.println(httpResponseCode);
    }
    
    // Finaliza la conexión
    http.end();
  } else {
    Serial.println("No conectado a WiFi.");
  }
  
  // Espera 10 segundos antes de realizar otra solicitud
  delay(10000);
}