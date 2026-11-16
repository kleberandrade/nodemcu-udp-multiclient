#!/usr/bin/env python3
"""Version 1 UDP codec and host sender/receiver; uses only the standard library."""
import argparse
import math
import socket
import struct
from dataclasses import dataclass

WIRE = struct.Struct("!4sBBI32sf")


@dataclass(frozen=True)
class Packet:
    id: int
    local: str
    distance: float
    enabled: bool = True


def encode(packet):
    label = packet.local.encode("ascii")
    if (not 1 <= packet.id <= 0xFFFFFFFF or len(label) > 31
            or any(c < 32 or c > 126 for c in label)
            or not math.isfinite(packet.distance) or packet.distance < 0
            or not isinstance(packet.enabled, bool)):
        raise ValueError("Invalid packet fields")
    try:
        return WIRE.pack(b"NCUD", 1, int(packet.enabled), packet.id,
                         label, packet.distance)
    except (OverflowError, struct.error) as exc:
        raise ValueError("Distance must fit in binary32") from exc


def decode(data):
    if len(data) != WIRE.size:
        raise ValueError("Expected exactly 46 bytes")
    magic, version, enabled, client_id, label, distance = WIRE.unpack(data)
    if magic != b"NCUD" or version != 1 or enabled not in (0, 1):
        raise ValueError("Invalid header")
    end = label.find(b"\0")
    if end < 0 or any(label[end:]):
        raise ValueError("Location must be terminated and zero-padded")
    try:
        packet = Packet(client_id, label[:end].decode("ascii"), distance, bool(enabled))
        encode(packet)  # Share field validation with the encoder.
    except (UnicodeError, ValueError) as exc:
        raise ValueError("Invalid packet fields") from exc
    return packet


def receive(sock):
    """Receive one complete datagram; caller sets timeout and handles ValueError."""
    data, address = sock.recvfrom(65535)
    return decode(data), address


def positive(value):
    result = int(value)
    if result <= 0:
        raise argparse.ArgumentTypeError("must be positive")
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest="command", required=True)
    receiver = commands.add_parser("receive", help="print valid packets as they arrive")
    receiver.add_argument("--bind", default="0.0.0.0")
    receiver.add_argument("--count", type=positive, default=None,
                          help="exit after this many valid packets (default: forever)")
    sender = commands.add_parser("send", help="send finite telemetry samples")
    sender.add_argument("--host", default="192.168.4.1")
    sender.add_argument("--id", type=positive, default=1)
    sender.add_argument("--local", default="host demo")
    sender.add_argument("--distance", type=float, default=42.5)
    sender.add_argument("--disabled", action="store_true")
    sender.add_argument("--count", type=positive, default=1)
    commands.add_parser("demo", help="exchange three clients over loopback, without boards")
    args = parser.parse_args()
    if args.command == "demo":
        # Separate sockets model three endpoints, not three Wi-Fi stations.
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as server:
            server.bind(("127.0.0.1", 0))
            server.settimeout(2)
            for client_id in range(1, 4):
                with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as client:
                    client.sendto(encode(Packet(client_id, "host demo", 42.5)),
                                  server.getsockname())
                    packet, address = receive(server)
                    if packet.id != client_id:
                        raise RuntimeError("Unexpected client ID")
                    print(f"{address[0]}:{address[1]} {packet}")
        return
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
        if args.command == "send":
            try:
                data = encode(Packet(args.id, args.local, args.distance, not args.disabled))
            except (ValueError, UnicodeError) as exc:
                parser.error(str(exc))
            for _ in range(args.count):
                sock.sendto(data, (args.host, 9999))
            print(f"Sent {args.count} packet(s); UDP has no delivery acknowledgement")
        else:
            sock.bind((args.bind, 9999))
            print(f"Listening on {args.bind}:9999", flush=True)
            count = 0
            while args.count is None or count < args.count:
                try:
                    packet, address = receive(sock)
                except ValueError as exc:
                    print(f"Rejected packet: {exc}", flush=True)
                    continue
                print(f"{address[0]}:{address[1]} {packet}", flush=True)
                count += 1


if __name__ == "__main__":
    main()
