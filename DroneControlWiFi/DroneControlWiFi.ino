#include <WiFi.h>
#include <WiFiServer.h>
#include <HardwareSerial.h>
#include <string.h>
#include <stdlib.h>

// Wi-Fi Configuration
const char* ssid = "DronESP"; // SSID (Wi-Fi network name)
const char* password = "12345678"; // Wi-Fi password

WiFiServer server(80); // Create a server that listens on port 80

// UART Configuration for F405 (Flight Controller)
#define RX_PIN 16 // RX pin on ESP32 (connected to TX2 on F405)
#define TX_PIN 17 // TX pin on ESP32 (connected to RX2 on F405)
HardwareSerial SerialDrone(2); // Use UART2 on ESP32 for communication with F405

// SBUS Frame Configuration
#define SBUS_FRAME_SIZE 25
uint8_t sbusFrame[SBUS_FRAME_SIZE];

// Default channel values
uint16_t channels[16] = {1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000,
                         1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000};

// Function to create SBUS frame
void createSBUSFrame(uint16_t channels[16], int failsafe, int frameLost) {
    memset(sbusFrame, 0, SBUS_FRAME_SIZE); // Ensure frame starts empty
    sbusFrame[0] = 0x0F; // Start byte

    // Pack channels into SBUS frame
    uint8_t bitIndex = 0;
    for (int i = 0; i < 16; i++) {
        uint16_t value = channels[i] & 0x07FF; // Mask to 11 bits
        for (int bit = 0; bit < 11; bit++) {
            if (value & (1 << bit)) {
                sbusFrame[1 + (bitIndex / 8)] |= (1 << (bitIndex % 8));
            } else {
                sbusFrame[1 + (bitIndex / 8)] &= ~(1 << (bitIndex % 8));
            }
            bitIndex++;
        }
    }

    sbusFrame[23] = 0x00; // Flags byte
    if (failsafe) {
        sbusFrame[23] |= (1 << 3);
    }
    if (frameLost) {
        sbusFrame[23] |= (1 << 2);
    }

    sbusFrame[24] = 0x00; // End byte
}

// Handle incoming commands from client and set channels
void handleClientCommand(char* command, uint16_t* channels) {
    if (strncmp(command, "CH", 2) == 0) {
        char* equalSign = strchr(command, '=');
        if (equalSign) {
            int channel = atoi(command + 2);
            int value = atoi(equalSign + 1);
            if (channel >= 1 && channel <= 16) {
                channels[channel - 1] = value;
            }
        }
    }

    // Ensure channels not explicitly set remain at 1000 (idle value)
    for (int i = 0; i < 16; i++) {
        if (channels[i] == 0) {
            channels[i] = 1000;
        }
    }
}

void setup() {
    // Initialize Serial Monitor for debugging
    Serial.begin(9600);
    Serial.println("ESP32 Starting...");

    // Initialize UART2 for communication with F405 flight controller
    SerialDrone.begin(100000, SERIAL_8E2, RX_PIN, TX_PIN); // SBUS uses 100 kbaud, 8E2

    // Set up Wi-Fi as Access Point (AP mode)
    WiFi.softAP(ssid, password);
    Serial.println("Access Point started!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.softAPIP()); // Print ESP32's IP address

    // Start the server
    server.begin();
    Serial.println("Server started on port 80.");
}

void loop() {
    // Wait for incoming client connection
    WiFiClient client = server.available();
    if (client) {
        Serial.println("Client connected.");

        char buffer[128];

        while (client.connected()) {
            // If client sends data (command from the mobile app)
            if (client.available()) {
                int length = client.readBytesUntil('\n', buffer, sizeof(buffer) - 1);
                buffer[length] = '\0'; // Null-terminate the string
                Serial.printf("Received command: %s\n", buffer);

                // Parse the command into channel values (example format: "CH1=1500")
                handleClientCommand(buffer, channels);

                // Create SBUS frame and send to F405
                createSBUSFrame(channels, 0, 0);
                SerialDrone.write(sbusFrame, SBUS_FRAME_SIZE);

                // For debugging, print SBUS frame
                Serial.print("SBUS Frame: ");
                for (int i = 0; i < SBUS_FRAME_SIZE; i++) {
                    Serial.printf("%02X ", sbusFrame[i]);
                }
                Serial.println();
            }

            // If F405 sends data back (e.g., status updates)
            if (SerialDrone.available()) {
                int length = SerialDrone.readBytesUntil('\n', buffer, sizeof(buffer) - 1);
                buffer[length] = '\0'; // Null-terminate the string
                Serial.printf("Received from F405: %s\n", buffer);

                // Send the F405 data back to the mobile app
                client.println(buffer);
            }
        }

        // Client disconnected
        client.stop();
        Serial.println("Client disconnected.");
    }

    // Periodic SBUS transmission (optional, for test purposes)
    static uint32_t lastSendTime = 0;
    if (millis() - lastSendTime > 14) { // SBUS frames are sent every 14ms
        createSBUSFrame(channels, 0, 0);
        SerialDrone.write(sbusFrame, SBUS_FRAME_SIZE);
        lastSendTime = millis();
    }
}
