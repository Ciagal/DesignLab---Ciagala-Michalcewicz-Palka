#include "wifiesp.h"

WiFiServer server(23);
WiFiClient client;

void initWiFi() {
  Serial.begin(115200);
  WiFi.softAP("DroneControl", "drone123");
  server.begin();
  Serial.println("Wi-Fi uruchomione");
}

void handleWiFi() {
  if (server.hasClient()) {
    client = server.available();
    Serial.println("Klient połączony.");
  }

  if (client && client.connected() && client.available()) {
    String command = client.readString();
    Serial.println("Odebrano komendę: " + command);
    // Możesz tu dodać przekazywanie komend do UART
  }
}
