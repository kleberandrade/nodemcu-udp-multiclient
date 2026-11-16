#pragma once

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>

namespace protocol {
constexpr size_t wireSize = 46;
struct Packet {
  uint32_t id;
  char local[32];
  float distance;
  bool enabled;
};

static_assert(sizeof(float) == 4 && std::numeric_limits<float>::is_iec559,
              "Protocol requires IEEE-754 binary32");

inline void write32(uint8_t* out, uint32_t value) {
  for (size_t i = 0; i < 4; ++i) out[i] = uint8_t(value >> (24 - i * 8));
}
inline uint32_t read32(const uint8_t* in) {
  return (uint32_t(in[0]) << 24) | (uint32_t(in[1]) << 16) |
         (uint32_t(in[2]) << 8) | uint32_t(in[3]);
}
inline bool valid(const Packet& packet) {
  const char* end = static_cast<const char*>(std::memchr(packet.local, 0, 32));
  if (packet.id == 0 || !end || !std::isfinite(packet.distance) ||
      packet.distance < 0) return false;
  // Printable ASCII keeps serial output safe and the wire format unambiguous.
  for (const char* p = packet.local; p != end; ++p)
    if (*p < 32 || *p > 126) return false;
  return true;
}
inline bool encode(const Packet& packet, uint8_t* out, size_t size) {
  if (!out || size != wireSize || !valid(packet)) return false;
  std::memset(out, 0, wireSize);
  std::memcpy(out, "NCUD", 4);
  out[4] = 1;
  out[5] = packet.enabled ? 1 : 0;
  write32(out + 6, packet.id);
  std::memcpy(out + 10, packet.local, std::strlen(packet.local));
  uint32_t bits;
  std::memcpy(&bits, &packet.distance, 4);
  write32(out + 42, bits);
  return true;
}
inline bool decode(const uint8_t* data, size_t size, Packet& out) {
  if (!data || size != wireSize || std::memcmp(data, "NCUD", 4) != 0 ||
      data[4] != 1 || data[5] > 1) return false;
  Packet candidate{};
  candidate.id = read32(data + 6);
  candidate.enabled = data[5] == 1;
  std::memcpy(candidate.local, data + 10, 32);
  const uint32_t bits = read32(data + 42);
  std::memcpy(&candidate.distance, &bits, 4);
  if (!valid(candidate)) return false;
  const size_t length = std::strlen(candidate.local);
  for (size_t i = length; i < 32; ++i)
    if (candidate.local[i] != 0) return false;
  out = candidate;
  return true;
}
}  // namespace protocol
