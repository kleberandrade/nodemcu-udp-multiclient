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
//UDP_PACKET packet = {1, "room", 0, true};
UDP_PACKET packet = {2, "kitchen", 0, false};

void setup()
{
  Serial.begin(115200);
  Serial.println();

  WiFi.mode(WIFI_STA);
  Serial.printf("Connecting to %s ", SERVER_NAME);
  WiFi.begin(SERVER_NAME, SERVER_PASWD);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println(" connected");
  Udp.begin(SERVER_PORT);
}

void loop()
{
  packet.distance = random(0, 1000);

  Udp.beginPacket(AP_SERVER, SERVER_PORT);
  Udp.write((byte *)&packet, sizeof(UDP_PACKET));
  Udp.endPacket();
  delay(1000);
}
