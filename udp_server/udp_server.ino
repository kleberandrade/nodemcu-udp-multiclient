#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const unsigned int SERVER_PORT  = 9999;
const char *SERVER_NAME         = "NODEMCU";
const char *SERVER_PASWD        = "";
const int SERVER_CHANNEL        = 4;

IPAddress AP_SERVER(192, 168, 4, 1);

typedef struct _udp_message {
  int id;
  int distance;
} UDP_DATA;

WiFiUDP Udp;
char incomingPacket[255];
char  replyPacekt[] = "acknowledged";

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

    // read the packet into packetBufffer
    int len = Udp.read(incomingPacket, 255);
    if (len > 0) {
      incomingPacket[len] = 0;
    }

    Serial.println("Contents: ");
    Serial.println(incomingPacket);
  }
}

void write(char *data) {
  // send a reply, to the IP address and port that sent us the packet we received
  Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
  Udp.write(data);
  Udp.endPacket();
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
