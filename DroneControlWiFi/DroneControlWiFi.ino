#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>

// Konfiguracja Access Point
const char* ssid = "DronESP";       // Nazwa sieci Wi-Fi
const char* password = "12345678";  // Hasło do sieci Wi-Fi (musi mieć co najmniej 8 znaków)

WiFiServer server(80);  // Tworzenie serwera na porcie 80

// Konfiguracja UART
#define RX_PIN 16  // Pin RX ESP32 (podłączony do TX2 na F405)
#define TX_PIN 17  // Pin TX ESP32 (podłączony do RX2 na F405)
HardwareSerial SerialDrone(2);  // UART2 na ESP32

void setup() {
  // Inicjalizacja portu szeregowego do debugowania
  Serial.begin(115200);
  Serial.println("Start config...");

  // Konfiguracja UART2
  SerialDrone.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);

  // Tworzenie Access Point
  WiFi.softAP(ssid, password);
  Serial.println("Access Point!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());  // Wyświetlanie lokalnego adresu IP (domyślnie 192.168.4.1)

  // Start serwera
  server.begin();
  Serial.println("Serwer uruchomiony.");
}

void loop() {
  // Obsługa połączeń Wi-Fi
  WiFiClient client = server.available();
  if (client) {
    Serial.println("Nowe połączenie!");

    while (client.connected()) {
      // Sprawdzanie, czy klient przesłał dane
      if (client.available()) {
        String data = client.readStringUntil('\n');
        Serial.println("Odebrano z Wi-Fi: " + data);

        // Wysyłanie danych do mikrokontrolera F405
        SerialDrone.println(data);
      }

      // Sprawdzanie, czy F405 przesłał dane
      if (SerialDrone.available()) {
        String droneData = SerialDrone.readStringUntil('\n');
        Serial.println("Odebrano z F405: " + droneData);

        // Wysyłanie danych z F405 do aplikacji Wi-Fi
        client.println(droneData);
      }
    }
    client.stop();
    Serial.println("Połączenie zakończone.");
  }
}
