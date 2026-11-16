// Keep the original sketch entry points; PlatformIO builds one role at a time.
// Explicit includes also let PlatformIO discover dependencies of both sketches.
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Protocol.h>
#ifdef UDP_SERVER
#include "../udp_server/udp_server.ino"
#else
#include "../udp_client/udp_client.ino"
#endif
