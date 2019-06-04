#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const unsigned int SERVER_PORT  = 9999;
const char *SERVER_NAME         = "NODEMCU";
const char *SERVER_PASWD        = "";
const int SERVER_CHANNEL        = 4;

IPAddress AP_SERVER(192, 168, 4, 1);

typedef struct _udp_packet {
  int id;
  char local[32];
  float distance;
  bool enabled;
} UDP_PACKET;

WiFiUDP Udp;
UDP_PACKET packet;

void setup() {
  Serial.begin(115200);
  Serial.println();

  WiFi.disconnect();
  WiFi.enableAP(false);
  WiFi.softAP(SERVER_NAME, SERVER_PASWD, SERVER_CHANNEL);
  delay(1000);

  Serial.println("Connected to wifi");
  printWifiStatus();

  Serial.println("Starting connection to server...");
  Udp.begin(SERVER_PORT);
}

void loop() {
  // if there's data available, read a packet
  int packetSize = Udp.parsePacket();
  if (packetSize)
  {
    Serial.print("Received packet of size ");
    Serial.println(packetSize);
    Serial.print("From ");
    IPAddress remoteIp = Udp.remoteIP();
    Serial.print(remoteIp);
    Serial.print(", port ");
    Serial.println(Udp.remotePort());

    int len = Udp.read((byte *)&packet, sizeof(UDP_PACKET));
    if (len > 0) {
      incomingPacket[len] = 0;
    }

    Serial.print("Contents: ");
    Serial.print("id = ");
    Serial.print(packet.id);
    Serial.print(", local = ");
    Serial.print(packet.local);
    Serial.print(", distance = ");
    Serial.print(packet.distance);
    Serial.print(", enabled = ");
    Serial.println(packet.enabled);
  }
}

void printWifiStatus() {
  Serial.print("SSID: ");
  Serial.println(SERVER_NAME);

  IPAddress ip = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(ip);

  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
