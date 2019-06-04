# NodeMCU UDP Multiclients

UDP server with multiclient with NodeMCU ESP8266

## How it works

Required libraries

```arduino
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
```

Structure of the message sent between the client and the server

```arduino
typedef struct _udp_packet {
    int id;
    char local[32];
    float distance;
    bool enabled;
} UDP_PACKET;
```

Declaration of constants

```arduino
const unsigned int SERVER_PORT  = 9999;
const char *SERVER_NAME         = "NODEMCU";
const char *SERVER_PASWD        = "";
const int SERVER_CHANNEL        = 4;
```

Declaration of variables

```arduino
WiFiUDP Udp;
UDP_PACKET packet;
```

Server configuration (Access Point - AP)

```arduino
void setup() {
    Serial.begin(115200);
    WiFi.disconnect();
    WiFi.enableAP(false);
    WiFi.softAP(SERVER_NAME, SERVER_PASWD, SERVER_CHANNEL);
    Udp.begin(SERVER_PORT);
}
```

Server reading message from client

```arduino
int packetSize = Udp.parsePacket();
if (packetSize) {
    Udp.read((byte *)&packet, sizeof(UDP_PACKET));
}
```

Client configuration (Station - STA)

```arduino
void setup()
{
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    WiFi.begin(SERVER_NAME, SERVER_PASWD);
    while (WiFi.status() != WL_CONNECTED) { delay(500); }
    Udp.begin(SERVER_PORT);
}
```

Client writing message to server

```arduino
Udp.beginPacket(AP_SERVER, SERVER_PORT);
Udp.write((byte *)&packet, sizeof(UDP_PACKET));
Udp.endPacket();
```

## Example

<p align="left">
  <img src="https://github.com/kleberandrade/nodemcu-udp-multiclient/blob/master/figures/sample.jpeg" height="400"/>
</p>

## Code Usage

[![](http://img.youtube.com/vi/j2EpkgpPG8g/0.jpg)](http://www.youtube.com/watch?v=j2EpkgpPG8g "")


## References

- [ESP8266WiFi library](https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/readme.html)
- [NodeMCU Documentation](https://nodemcu.readthedocs.io/en/master/modules/wifi/#wifisetmode) 

## License

    Copyright 2017-2019 Kleber de Oliveira Andrade

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.