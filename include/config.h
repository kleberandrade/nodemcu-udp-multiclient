#pragma once

#if __has_include("config.local.h")
#include "config.local.h"
#endif

#ifndef WIFI_SSID
#define WIFI_SSID "NODEMCU"
#endif
#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "nodemcu-demo"
#endif
#ifndef CLIENT_ID
#define CLIENT_ID 1
#endif
#ifndef CLIENT_LOCATION
#define CLIENT_LOCATION "living room"
#endif

namespace config {
constexpr unsigned short port = 9999;
constexpr unsigned char channel = 4;
constexpr unsigned char maxClients = 4;
constexpr unsigned long sendIntervalMs = 1000;
static_assert(sizeof(WIFI_SSID) > 1 && sizeof(WIFI_SSID) <= 33,
              "SSID must contain 1 to 32 bytes");
static_assert(sizeof(WIFI_PASSWORD) == 1 ||
                  (sizeof(WIFI_PASSWORD) >= 9 && sizeof(WIFI_PASSWORD) <= 64),
              "Use an empty password or 8 to 63 bytes");
static_assert(CLIENT_ID > 0 && CLIENT_ID <= 0xffffffffULL,
              "Client ID must be a nonzero uint32");
static_assert(sizeof(CLIENT_LOCATION) <= 32, "Location must fit in 31 bytes");
}  // namespace config
