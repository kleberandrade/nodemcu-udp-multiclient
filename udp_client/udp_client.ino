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
  Udp.beginPacket(AP_SERVER, SERVER_PORT);
  Udp.write(replyPacekt);
  Udp.endPacket();
  delay(1000);
}
