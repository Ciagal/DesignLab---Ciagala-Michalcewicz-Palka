#include <WiFi.h>
#include <WiFiServer.h>
#include <HardwareSerial.h>
#include <string.h>
#include <stdlib.h>

// Konfiguracja Wi-Fi
const char* ssid = "DronESP";          // Nazwa sieci Wi-Fi
const char* password = "12345678";     // Hasło do Wi-Fi

WiFiServer server(80);                 // Serwer na porcie 80

// UART dla SBUS
#define RX_PIN 16                     // Pin RX na ESP32 (połączony z TX kontrolera lotu)
#define TX_PIN 17                     // Pin TX na ESP32 (połączony z RX kontrolera lotu)
HardwareSerial SerialDrone(2);        // Użycie UART2 na ESP32

// Ramka SBUS
#define SBUS_FRAME_SIZE 25
uint8_t sbusFrame[SBUS_FRAME_SIZE];

// Wartości kanałów - domyślnie 1000
uint16_t channels[16] = {1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000,
                         1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000};

// Funkcja mapowania wartości kanałów na SBUS (880-2112)
uint16_t mapToSBUS(uint16_t value) {
    return map(value, 1000, 2000, 880, 2112);
}

// Funkcja tworzenia ramki SBUS
void createSBUSFrame(uint16_t channels[16], int failsafe, int frameLost) {
    memset(sbusFrame, 0, SBUS_FRAME_SIZE);  // Wyczyszczenie ramki
    sbusFrame[0] = 0x0F;                    // Start byte SBUS

    uint8_t bitIndex = 0;
    for (int i = 0; i < 16; i++) {
        uint16_t sbusValue = mapToSBUS(channels[i]);  // Mapowanie kanałów na SBUS
        for (int bit = 0; bit < 11; bit++) {
            if (sbusValue & (1 << bit)) {
                sbusFrame[1 + (bitIndex / 8)] |= (1 << (bitIndex % 8));
            }
            bitIndex++;
        }
    }

    // Ustawienie flag
    sbusFrame[23] = 0x00;
    if (failsafe) sbusFrame[23] |= (1 << 3);
    if (frameLost) sbusFrame[23] |= (1 << 2);
    sbusFrame[24] = 0x00;  // End byte SBUS
}

// Obsługa komend klienta
void handleClientCommand(char* command, uint16_t* channels) {
    if (strncmp(command, "CH", 2) == 0) {
        char* equalSign = strchr(command, '=');
        if (equalSign) {
            int channel = atoi(command + 2);  // Numer kanału
            int value = atoi(equalSign + 1);  // Wartość kanału

            if (channel >= 1 && channel <= 16) {
                channels[channel - 1] = constrain(value, 1000, 2000);
            }
        }
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("ESP32 SBUS Controller - Start");

    // Konfiguracja UART2 dla SBUS (8E2 - 100000 baud)
    SerialDrone.begin(100000, SERIAL_8E2, RX_PIN, TX_PIN);

    // Tryb Access Point
    WiFi.softAP(ssid, password);
    Serial.print("Access Point IP: ");
    Serial.println(WiFi.softAPIP());

    server.begin();
    Serial.println("Serwer uruchomiony na porcie 80.");
}

void loop() {
    WiFiClient client = server.available();
    if (client) {
        Serial.println("Klient podłączony.");
        char buffer[128];

        while (client.connected()) {
            // Odbieranie komend od klienta
            if (client.available()) {
                int length = client.readBytesUntil('\n', buffer, sizeof(buffer) - 1);
                buffer[length] = '\0';
                Serial.printf("Odebrano: %s\n", buffer);

                handleClientCommand(buffer, channels);

                // Tworzenie i wysyłanie ramki SBUS
                createSBUSFrame(channels, 0, 0);
                SerialDrone.write(sbusFrame, SBUS_FRAME_SIZE);

                // Debugowanie - drukowanie ramki SBUS
                Serial.print("SBUS Frame: ");
                for (int i = 0; i < SBUS_FRAME_SIZE; i++) {
                    Serial.printf("%02X ", sbusFrame[i]);
                }
                Serial.println();
            }

            // Debugowanie danych z kontrolera lotu
            if (SerialDrone.available()) {
                int length = SerialDrone.readBytesUntil('\n', buffer, sizeof(buffer) - 1);
                buffer[length] = '\0';
                Serial.printf("Odebrano z FC: %s\n", buffer);
                client.println(buffer);
            }
        }

        client.stop();
        Serial.println("Klient rozłączony.");
    }

    // Wysyłanie ramki SBUS co 14 ms (standard SBUS)
    static uint32_t lastSendTime = 0;
    if (millis() - lastSendTime > 14) {
        createSBUSFrame(channels, 0, 0);
        SerialDrone.write(sbusFrame, SBUS_FRAME_SIZE);
        lastSendTime = millis();
    }
}
