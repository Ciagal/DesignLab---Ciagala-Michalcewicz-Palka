/***************************************************************
 * ESP32 Drone SBUS Controller (Dual Server)
 *
 * - AP "DronESP" (pw "12345678").
 * - serverHTTP (port 80): old HTTP model (for application).
 * - serverRaw (port 81): continuous RAW session (for PuTTY).
 *
 * - SBUS -> FC (TX=17, RX=-1).
 ***************************************************************/

#include <WiFi.h>
#include <WiFiServer.h>
#include <HardwareSerial.h>
#include <string.h>
#include <stdlib.h>

const char* ssid     = "DronESP";
const char* password = "12345678";

WiFiServer serverHTTP(80);
WiFiServer serverRaw(81);

#define SBUS_RX_PIN -1
#define SBUS_TX_PIN 17

HardwareSerial SerialDrone(2);

#define SBUS_FRAME_SIZE 25
uint8_t sbusFrame[SBUS_FRAME_SIZE];

uint16_t channels[16] = {
  1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000,
  1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000
};

uint16_t mapToSBUS(uint16_t value) {
  return map(value, 1000, 2000, 880, 2112);
}

void createSBUSFrame(const uint16_t* ch, bool failsafe, bool frameLost) {
  memset(sbusFrame, 0, SBUS_FRAME_SIZE);
  sbusFrame[0] = 0x0F;

  uint8_t bitIndex = 0;
  for (int i = 0; i < 16; i++) {
    uint16_t sbusValue = mapToSBUS(ch[i]);
    for (int bit = 0; bit < 11; bit++) {
      if (sbusValue & (1 << bit)) {
        sbusFrame[1 + (bitIndex / 8)] |= (1 << (bitIndex % 8));
      }
      bitIndex++;
    }
  }

  if (failsafe)  sbusFrame[23] |= (1 << 3);
  if (frameLost) sbusFrame[23] |= (1 << 2);

  sbusFrame[24] = 0x00;
}

void handleClientCommand(const char* command, uint16_t* chArray) {
  if (strncmp(command, "CH", 2) == 0) {
    const char* eq = strchr(command, '=');
    if (eq) {
      int channel = atoi(command + 2);
      int value   = atoi(eq + 1);
      if (channel >= 1 && channel <= 16) {
        chArray[channel - 1] = constrain(value, 1000, 2000);
      }
    }
  }
}

void handleHTTP(WiFiClient client) {
  String fullRequest;
  unsigned long startTime = millis();
  while (millis() - startTime < 1000) {
    while (client.available()) {
      char c = client.read();
      fullRequest += c;
      startTime = millis();
    }
    delay(5);
  }

  Serial.println("HTTP Request:\n-----------");
  Serial.println(fullRequest);
  Serial.println("-----------");

  int startIndex = 0;
  while (true) {
    int newLinePos = fullRequest.indexOf('\n', startIndex);
    if (newLinePos == -1) {
      break;
    }
    String line = fullRequest.substring(startIndex, newLinePos);
    line.trim();
    startIndex = newLinePos + 1;

    char cmdBuff[128];
    line.toCharArray(cmdBuff, sizeof(cmdBuff));

    handleClientCommand(cmdBuff, channels);
  }

  createSBUSFrame(channels, false, false);
  SerialDrone.write(sbusFrame, SBUS_FRAME_SIZE);

  Serial.print("SBUS Frame: ");
  for (int i = 0; i < SBUS_FRAME_SIZE; i++) {
    Serial.printf("%02X ", sbusFrame[i]);
  }
  Serial.println();

  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/plain");
  client.println("Connection: close");
  client.println();
  client.println("OK: command parsed");

  client.flush();
  client.stop();
  Serial.println("HTTP client disconnected.");
}

void handleRaw(WiFiClient client) {
  Serial.println("Raw client connected (port 81).");
  client.println("Welcome to SBUS Raw console! Type 'CH3=1500' etc.\n");

  while (client.connected()) {
    if (client.available()) {
      String line = client.readStringUntil('\n');
      line.trim();
      Serial.printf("[RAW] line: %s\n", line.c_str());

      handleClientCommand(line.c_str(), channels);

      createSBUSFrame(channels, false, false);
      SerialDrone.write(sbusFrame, SBUS_FRAME_SIZE);

      client.println("OK: command parsed");
    }

    static uint32_t lastRawSend = 0;
    if (millis() - lastRawSend > 14) {
      createSBUSFrame(channels, false, false);
      SerialDrone.write(sbusFrame, SBUS_FRAME_SIZE);
      lastRawSend = millis();
    }

    delay(1);
  }

  client.stop();
  Serial.println("Raw client disconnected (port 81).");
}

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 SBUS Controller - Start (Dual server)");

  SerialDrone.begin(100000, SERIAL_8E2, SBUS_RX_PIN, SBUS_TX_PIN, true);

  WiFi.softAP(ssid, password);
  Serial.print("Access Point IP: ");
  Serial.println(WiFi.softAPIP());

  serverHTTP.begin();
  Serial.println("HTTP server started on port 80.");

  serverRaw.begin();
  Serial.println("Raw server started on port 81.");
}

void loop() {
  WiFiClient clientHTTP = serverHTTP.available();
  if (clientHTTP) {
    handleHTTP(clientHTTP);
  }

  WiFiClient clientR = serverRaw.available();
  if (clientR) {
    handleRaw(clientR);
  }

  static uint32_t lastSendTime2 = 0;
  if (millis() - lastSendTime2 > 14) {
    createSBUSFrame(channels, false, false);
    SerialDrone.write(sbusFrame, SBUS_FRAME_SIZE);
    lastSendTime2 = millis();
  }
}
