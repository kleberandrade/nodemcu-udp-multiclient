#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Protocol.h>
#include "config.h"

WiFiUDP udp;
bool listening = false;
unsigned long lastSent = 0;
unsigned long lastStatus = 0;
protocol::Packet packet = {CLIENT_ID, CLIENT_LOCATION, 0, true};

void setup() {
  Serial.begin(115200);
  Serial.println();
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.printf("Connecting to %s as client %lu\n", WIFI_SSID,
                static_cast<unsigned long>(packet.id));
}

void loop() {
  const unsigned long now = millis();
  if (WiFi.status() != WL_CONNECTED) {
    if (listening) { udp.stop(); listening = false; }
    if (now - lastStatus >= 5000) {
      Serial.println("Waiting for Wi-Fi (automatic reconnect enabled)");
      lastStatus = now;
    }
    delay(10);
    return;
  }
  if (!listening) {
    listening = udp.begin(config::port) == 1;
    Serial.printf("Client IP %s; UDP %s\n", WiFi.localIP().toString().c_str(),
                  listening ? "ready" : "bind failed; retrying");
    if (!listening) { delay(1000); return; }
  }
  if (now - lastSent >= config::sendIntervalMs) {
    lastSent = now;
    packet.distance = static_cast<float>(random(0, 1000));
    uint8_t buffer[protocol::wireSize];
    if (!protocol::encode(packet, buffer, sizeof(buffer))) {
      Serial.println("Invalid client fields; check config");
    } else if (!udp.beginPacket(WiFi.gatewayIP(), config::port) ||
               udp.write(buffer, sizeof(buffer)) != sizeof(buffer) ||
               !udp.endPacket()) {
      Serial.println("UDP send failed");
    } else {
      Serial.printf("Sent id=%lu distance=%.2f\n",
                    static_cast<unsigned long>(packet.id), packet.distance);
    }
  }
  delay(1);
}
