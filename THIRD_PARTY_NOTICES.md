# Third-party notices

The project source is MIT licensed; see LICENSE. Original source and demonstration
assets are attributed to Kléber de Oliveira Andrade, including the original
README's 2017–2019 credit. No existing license text has been removed or replaced.

Dependencies are downloaded by PlatformIO and are not vendored into this repository:

- [ESP8266 Arduino core 3.1.2](https://github.com/esp8266/Arduino/tree/3.1.2),
  including ESP8266WiFi and WiFiUDP: LGPL-2.1 and component-specific notices.
  Its SDK, lwIP, and other bundled components retain their original notices.
- [PlatformIO Core](https://github.com/platformio/platformio-core): Apache-2.0.
- [PlatformIO Espressif8266 platform](https://github.com/platformio/platform-espressif8266): Apache-2.0.
- Xtensa GCC toolchain: GCC and bundled runtime/library licenses and exceptions;
  consult the downloaded toolchain's license files when redistributing it.
- esptool and other upload/build utilities retain their own license notices in
  the downloaded packages.

Consult each upstream package's license files for the full applicable terms,
particularly when redistributing firmware or toolchains. The repository's MIT
license does not relicense those components.
