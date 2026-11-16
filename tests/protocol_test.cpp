#include <Protocol.h>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <limits>

int main(int argc, char** argv) {
  // Fixed fixture: id=0x01020304, local="room", distance=42.5, enabled=true.
  protocol::Packet sample{0x01020304, "room", 42.5f, true};
  uint8_t bytes[protocol::wireSize];
  assert(protocol::encode(sample, bytes, sizeof(bytes)));
  if (argc == 2 && std::strcmp(argv[1], "encode") == 0) {
    for (uint8_t byte : bytes) std::printf("%02x", byte);
    std::puts("");
    return 0;
  }
  if (argc == 3 && std::strcmp(argv[1], "decode") == 0) {
    if (std::strlen(argv[2]) != protocol::wireSize * 2) return 1;
    for (size_t i = 0; i < sizeof(bytes); ++i) {
      char pair[3]{argv[2][i * 2], argv[2][i * 2 + 1], 0};
      bytes[i] = static_cast<uint8_t>(std::strtoul(pair, nullptr, 16));
    }
    protocol::Packet packet{};
    if (!protocol::decode(bytes, sizeof(bytes), packet)) return 1;
    uint8_t result[protocol::wireSize];
    assert(protocol::encode(packet, result, sizeof(result)));
    assert(std::memcmp(bytes, result, sizeof(bytes)) == 0);
    return 0;
  }
  protocol::Packet out{};
  assert(protocol::decode(bytes, sizeof(bytes), out));
  assert(out.id == sample.id && out.distance == sample.distance && out.enabled);
  assert(std::strcmp(out.local, sample.local) == 0);
  assert(bytes[6] == 1 && bytes[9] == 4);
  assert(bytes[42] == 0x42 && bytes[43] == 0x2a);
  assert(!protocol::decode(nullptr, sizeof(bytes), out));
  assert(!protocol::encode(sample, nullptr, sizeof(bytes)));
  for (size_t length = 0; length < sizeof(bytes); ++length)
    assert(!protocol::decode(bytes, length, out));
  assert(!protocol::decode(bytes, sizeof(bytes) + 1, out));
  assert(!protocol::encode(sample, bytes, sizeof(bytes) - 1));
  sample.id = 0;
  assert(!protocol::encode(sample, bytes, sizeof(bytes)));
  sample.id = 0xffffffff;
  sample.distance = std::numeric_limits<float>::infinity();
  assert(!protocol::encode(sample, bytes, sizeof(bytes)));
  sample.distance = std::numeric_limits<float>::quiet_NaN();
  assert(!protocol::encode(sample, bytes, sizeof(bytes)));
  sample.distance = -1;
  assert(!protocol::encode(sample, bytes, sizeof(bytes)));
  sample.distance = 0;
  sample.enabled = false;
  std::memset(sample.local, 'A', 32);
  assert(!protocol::encode(sample, bytes, sizeof(bytes)));
  sample.local[31] = 0;
  assert(protocol::encode(sample, bytes, sizeof(bytes)));
  assert(protocol::decode(bytes, sizeof(bytes), out));
  assert(out.id == 0xffffffff && !out.enabled && out.distance == 0);
  sample.local[0] = '\n';
  assert(!protocol::encode(sample, bytes, sizeof(bytes)));
  // Failed decoding must never mutate the last valid output.
  const protocol::Packet before = out;
  bytes[4] = 2;
  assert(!protocol::decode(bytes, sizeof(bytes), out));
  assert(before.id == out.id && before.distance == out.distance &&
         before.enabled == out.enabled && std::strcmp(before.local, out.local) == 0);
  std::puts("C++ protocol checks passed");
}
