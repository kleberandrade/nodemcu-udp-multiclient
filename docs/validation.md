# Validation and hardware acceptance

## Automated scope

The upgrade was validated locally with Python 3.13.18 and Apple Clang 21 on
Apple Silicon macOS. The repository's canonical dependency versions are recorded
in `requirements.txt` and `platformio.ini`; no global PlatformIO installation is
required. Linux CI is configured but has not been run remotely as part of this
local-only change.

Commands, run from the repository root with the virtual environment activated:

```sh
python -m unittest discover -s tests -v
python examples/udp_demo.py demo
pio run -e server -e client -e client2 -e client3
git diff --check
```

Results:

- Seven Python unittest cases passed. The suite also compiles and executes C++
  assertions against the same header used by the firmware, under AddressSanitizer
  and UndefinedBehaviorSanitizer. No sanitizer failures were reported.
- C++ and Python agree on an independent fixed vector, maximum client ID, empty
  and maximum-length labels, float32 boundaries, both enabled states, and 32
  deterministic randomized valid packets.
- Negative cases cover all truncated lengths, oversized packets, malformed magic,
  unsupported versions, flags, ID zero, unterminated/non-ASCII/control-character
  labels, invalid padding, NaN, infinity, and negative distance. A failed native
  decode leaves its previous packet unchanged.
- The UDP integration case receives messages from three distinct loopback source
  sockets and continues after a malformed datagram.
- `demo` completed with IDs 1, 2, and 3. The documented `receive --count 3` and
  three separate `send` processes were also exercised over loopback, with all
  three IDs received and a successful receiver exit.
- The server and all three client configurations compiled into ESP8266 firmware
  binaries. These builds use the NodeMCU 1.0 board definition, Arduino core 3.1.2,
  and Xtensa GCC 10.3.0. Upstream `elf2bin.py` emitted Python 3.13 escape-sequence
  SyntaxWarnings; firmware generation completed successfully.
- `git diff --check` passed. The original MIT LICENSE file was preserved.

This is protocol behavior and compilation coverage, not a claim of complete
branch coverage or physical networking validation. In particular, loopback
endpoints do not simulate Wi-Fi station association, RF interference, the lwIP
queue under load, flash/upload behavior, or power interruptions.

## Hardware checklist (not yet executed)

1. Change the public demonstration credentials. Build and upload `server` to one
   NodeMCU board. At 115200 baud, confirm the AP address and `UDP 9999: ready`.
2. Upload `client`, `client2`, and `client3` to three other boards. Confirm each
   associates and the server prints all three distinct IDs at roughly 1 Hz.
   A computer joined to the AP occupies the fourth configured station slot.
3. With a computer joined to the AP, run the host sender with ID 100. Confirm
   its fields appear on the server. Send malformed and oversized datagrams;
   confirm rejection and that a subsequent valid message is still logged.
4. Power off the server for at least 10 seconds. Restore power and verify that
   clients reconnect, reopen UDP sockets, and resume sending without reset.
   Repeat with one client power-cycled independently.
5. Run for an extended period with stable power and inspect for resets, missing
   clients, and unexpectedly high packet loss. Do not infer reliable delivery
   from the absence of errors in client logs.
6. Repeat upload/reset with the actual USB bridges and operating systems used
   by the intended audience. Record board revision, power source, environment,
   and observed results before claiming hardware support has been tested.

No physical board was connected or tested during this upgrade. Upload commands,
Wi-Fi reconnection, and multi-station radio behavior remain manual validation
items. No push or remote workflow execution was performed.
