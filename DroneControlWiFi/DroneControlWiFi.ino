#include <WiFi.h>
#include <WiFiServer.h>
#include <HardwareSerial.h>

// Wi-Fi Configuration
const char* ssid = "DronESP";       // SSID (Wi-Fi network name)
const char* password = "12345678";  // Wi-Fi password

WiFiServer server(80);  // Create a server that listens on port 80

// UART Configuration for F405 (Flight Controller)
#define RX_PIN 16  // RX pin on ESP32 (connected to TX2 on F405)
#define TX_PIN 17  // TX pin on ESP32 (connected to RX2 on F405)
HardwareSerial SerialDrone(2);  // Use UART2 on ESP32 for communication with F405

void setup() {
  // Initialize Serial Monitor for debugging
  Serial.begin(115200);
  Serial.println("ESP32 Starting...");

  // Initialize UART2 for communication with F405 flight controller
  SerialDrone.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);

  // Set up Wi-Fi as Access Point (AP mode)
  WiFi.softAP(ssid, password);
  Serial.println("Access Point started!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());  // Print ESP32's IP address

  // Start the server
  server.begin();
  Serial.println("Server started on port 80.");
}

void loop() {
  // Wait for incoming client connection
  WiFiClient client = server.available();
  if (client) {
    Serial.println("Client connected.");

    while (client.connected()) {
      // If client sends data (command from the mobile app)
      if (client.available()) {
        String command = client.readStringUntil('\n');
        Serial.println("Received command: " + command);

        // Send the command to the F405 flight controller via UART
        SerialDrone.println(command); // Forward command to F405

        // For debugging, print command sent to the F405
        Serial.println("Sending to F405: " + command);
      }

      // If F405 sends data back (e.g., status updates)
      if (SerialDrone.available()) {
        String droneData = SerialDrone.readStringUntil('\n');
        Serial.println("Received from F405: " + droneData);

        // Send the F405 data back to the mobile app
        client.println(droneData);
      }
    }

    // Client disconnected
    client.stop();
    Serial.println("Client disconnected.");
  }
}
