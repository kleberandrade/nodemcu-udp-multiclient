# UDP protocol version 1

Each datagram is **exactly 46 bytes**, addressed to UDP port 9999. Integers and
floating-point bits are sent in network byte order (big-endian). The codec never
serializes a C++ struct's memory representation.

## Layout

- Bytes 0–3: ASCII magic `NCUD` (hex `4e 43 55 44`).
- Byte 4: version, exactly `01`.
- Byte 5: enabled flag, exactly `00` or `01`.
- Bytes 6–9: unsigned 32-bit ID, from 1 through 4294967295.
- Bytes 10–41: location (`local`), 0–31 printable ASCII bytes (`20`–`7e`),
  followed by a null terminator and zero padding through byte 41.
- Bytes 42–45: IEEE-754 binary32 distance. Must be finite and nonnegative;
  negative zero is accepted. The example uses simulated unitless values.

Receivers reject extra or missing bytes, unknown versions/magic, ID zero, invalid
flags, control/non-ASCII characters, a missing terminator, nonzero label padding,
NaN, infinity, and negative distance. A failed C++ decode leaves its destination
unchanged. No checksum, timestamp, acknowledgement, registration, or sequence
number is included. The server handles each accepted packet independently and
retains no per-client state.

## C++ API

`lib/UdpProtocol/src/Protocol.h` is header-only and independent of Arduino:

```cpp
#include <Protocol.h>

protocol::Packet packet{1, "room", 42.5f, true};
uint8_t bytes[protocol::wireSize];
if (protocol::encode(packet, bytes, sizeof(bytes))) {
  // Send the complete bytes array as one UDP datagram.
}
protocol::Packet decoded{};
if (protocol::decode(bytes, sizeof(bytes), decoded)) {
  // Read validated fields from decoded.
}
```

`encode(const Packet&, uint8_t*, size_t)` and
`decode(const uint8_t*, size_t, Packet&)` return `bool` and require a size equal to
`protocol::wireSize`. `Packet` has a `uint32_t id`, `char local[32]`, `float distance`,
and `bool enabled`. The implementation checks for IEEE-754 32-bit floats at
compile time and uses bounded validation before string operations. Encoding
canonicalizes bytes after the first label terminator to zero.

## Python API

`examples/udp_demo.py` exposes `Packet`, `encode(packet) -> bytes`,
`decode(data) -> Packet`, and `receive(socket) -> (Packet, address)`.
Malformed datagrams raise `ValueError`. Encoding an invalid ASCII label can also
raise `UnicodeEncodeError` (a `ValueError` subclass). Socket errors propagate to
the caller. A positive finite distance that does not fit binary32 is rejected.
The wire representation rounds Python floats to binary32.

## Shared test vector

For ID `16909060` (`0x01020304`), label `room`, distance `42.5`, enabled `true`:

```text
4e435544 01 01 01020304
726f6f6d 00000000 00000000 00000000 00000000
00000000 00000000 00000000
422a0000
```

Whitespace above is only for readability. There are 28 zero bytes after `room`.
Both language codecs are tested against this independently specified vector.
The original pre-upgrade raw struct format is unsupported; upgrade all endpoints.
