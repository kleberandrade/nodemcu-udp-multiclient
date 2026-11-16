"""Run with python -m unittest discover -s tests -v. Requires a C++ compiler."""
import os
from pathlib import Path
import random
import shlex
import socket
import struct
import subprocess
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "examples"))
from udp_demo import Packet, decode, encode, receive  # noqa: E402

GOLDEN = bytes.fromhex("4e435544010101020304" + "726f6f6d" + "00" * 28 + "422a0000")


class ProtocolTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        build = ROOT / ".build"
        build.mkdir(exist_ok=True)
        cls.binary = build / "protocol_test"
        cls.runtime_env = dict(os.environ, UBSAN_OPTIONS="halt_on_error=1")
        subprocess.run(shlex.split(os.environ.get("CXX", "c++")) + [
            "-std=c++11", "-Wall", "-Wextra", "-Werror",
            "-fsanitize=address,undefined", "-fno-omit-frame-pointer",
            "-I" + str(ROOT / "lib/UdpProtocol/src"),
            str(ROOT / "tests/protocol_test.cpp"), "-o", str(cls.binary)], check=True)

    def cpp_accepts(self, data):
        result = subprocess.run([str(self.binary), "decode", data.hex()],
                                capture_output=True, env=self.runtime_env)
        self.assertIn(result.returncode, (0, 1), result.stderr.decode())
        self.assertEqual(result.stderr, b"", "Native runtime reported a diagnostic")
        return result.returncode == 0

    def test_cpp_native_checks(self):
        subprocess.run([str(self.binary)], check=True, env=self.runtime_env)

    def test_shared_golden_vector(self):
        packet = Packet(0x01020304, "room", 42.5)
        self.assertEqual(encode(packet), GOLDEN)
        self.assertEqual(decode(GOLDEN), packet)
        encoded = subprocess.check_output([str(self.binary), "encode"], text=True,
                                          env=self.runtime_env)
        self.assertEqual(bytes.fromhex(encoded), GOLDEN)
        self.assertTrue(self.cpp_accepts(GOLDEN))

    def test_cross_language_boundaries(self):
        for packet in (Packet(1, "", 0, False),
                       Packet(0xFFFFFFFF, "A" * 31, 3.4028234663852886e38)):
            data = encode(packet)
            self.assertEqual(decode(data), packet)
            self.assertTrue(self.cpp_accepts(data))

    def test_malformed_datagrams(self):
        bad = [GOLDEN[:n] for n in range(len(GOLDEN))] + [GOLDEN + b"x"]
        for offset, value in ((0, 0), (4, 0), (4, 2), (5, 2), (10, 10), (10, 255),
                              (20, 1)):
            data = bytearray(GOLDEN)
            data[offset] = value
            bad.append(bytes(data))
        bad += [GOLDEN[:6] + b"\0" * 4 + GOLDEN[10:],
                GOLDEN[:10] + b"A" * 32 + GOLDEN[42:]]
        for value in (float("inf"), float("-inf"), float("nan"), -1.0):
            bad.append(GOLDEN[:42] + struct.pack("!f", value))
        for data in bad:
            with self.subTest(data=data.hex()):
                with self.assertRaises(ValueError):
                    decode(data)
                self.assertFalse(self.cpp_accepts(data))

    def test_invalid_fields(self):
        for packet in (Packet(0, "room", 1), Packet(2**32, "room", 1),
                       Packet(1, "A" * 32, 1), Packet(1, "bad\0label", 1),
                       Packet(1, "bad\nlabel", 1), Packet(1, "room", -1),
                       Packet(1, "room", float("nan")), Packet(1, "room", 1e40)):
            with self.subTest(packet=packet), self.assertRaises(ValueError):
                encode(packet)

    def test_random_binary32_interoperability(self):
        rng = random.Random(1234)
        for _ in range(32):
            packet = Packet(rng.randint(1, 0xFFFFFFFF), "kitchen",
                            struct.unpack("!f", struct.pack("!f", rng.uniform(0, 1000)))[0],
                            bool(rng.getrandbits(1)))
            data = encode(packet)
            self.assertEqual(decode(data), packet)
            self.assertTrue(self.cpp_accepts(data))

    def test_udp_multiple_endpoints_and_rejection_recovery(self):
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as server:
            server.bind(("127.0.0.1", 0))
            server.settimeout(2)
            clients = [socket.socket(socket.AF_INET, socket.SOCK_DGRAM) for _ in range(3)]
            try:
                clients[0].sendto(b"invalid", server.getsockname())
                with self.assertRaises(ValueError):
                    receive(server)
                for i, client in enumerate(clients, 1):
                    client.sendto(encode(Packet(i, "room", float(i))), server.getsockname())
                received = [receive(server) for _ in clients]
                self.assertEqual({p.id for p, _ in received}, {1, 2, 3})
                self.assertEqual(len({address for _, address in received}), 3)
            finally:
                for client in clients:
                    client.close()


if __name__ == "__main__":
    unittest.main()
