#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Protocol.h>
#include "config.h"

WiFiUDP udp;
bool listening = false;

void setup() {
  Serial.begin(115200);
  Serial.println();
  WiFi.persistent(false);
  WiFi.mode(WIFI_AP);
  const IPAddress address(192, 168, 4, 1);
  if (!WiFi.softAPConfig(address, address, IPAddress(255, 255, 255, 0)) ||
      !WiFi.softAP(WIFI_SSID, WIFI_PASSWORD, config::channel, false,
                   config::maxClients)) {
    Serial.println("Access point setup failed; check config and reset.");
    return;
  }
  listening = udp.begin(config::port) == 1;
  Serial.printf("AP %s at %s; UDP %u: %s\n", WIFI_SSID,
                WiFi.softAPIP().toString().c_str(), config::port,
                listening ? "ready" : "bind failed; reset board");
}

void loop() {
  if (!listening) { delay(100); return; }
  const int size = udp.parsePacket();
  if (size <= 0) { delay(1); return; }
  const IPAddress remote = udp.remoteIP();
  const uint16_t port = udp.remotePort();
  // One extra byte and an exact size check prevent accepting a truncated packet.
  uint8_t buffer[protocol::wireSize + 1];
  const int length = udp.read(buffer, sizeof(buffer));
  // parsePacket() releases any unread tail on its next call (ESP8266 core).
  protocol::Packet packet{};
  if (size != int(protocol::wireSize) || length != size ||
      !protocol::decode(buffer, size, packet)) {
    Serial.printf("Rejected UDP packet (%d bytes)\n", size);
    return;
  }
  Serial.printf("%s:%u id=%lu local=%s distance=%.2f enabled=%s\n",
                remote.toString().c_str(), port,
                static_cast<unsigned long>(packet.id), packet.local,
                packet.distance, packet.enabled ? "true" : "false");
}
