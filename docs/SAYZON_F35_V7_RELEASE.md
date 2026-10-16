# SAYZON 40 mm F-35 — v7 experimental engineering preview

**Status:** two-aircraft binding/control validation **PASS**; **NOT flight-ready**; this is an experimental development snapshot, not a stable firmware release.

**Primary discussion:** [avgbob/DIY-Multiprotocol-TX-Module PR #1](https://github.com/avgbob/DIY-Multiprotocol-TX-Module/pull/1)

## Tested firmware identity

- Original file: `f35v7.bin`
- Size: `116560` bytes
- SHA-256: `3d1f1361cb77f2878216e52c040e12828ad88cb7046fcdeaf403e28a7a235479`
- Embedded firmware version: MultiModule **1.3.4.74**, STM32, AIR build profile
- Tester hardware: **FrSky Taranis Q X7 + iRangeX IRX4 Plus external MultiModule**
- The tester confirms **this exact binary** was flashed during successful binding and control operation on the original and independently purchased replacement SAYZON F-35.
- Full control function on the replacement was reported operational; no firmware changes were required between aircraft.
- The replacement aircraft's **stock transmitter** has **not** yet been RF-captured; the current pairing identity and flight address should not be assumed universal.

### Source provenance and current repository state

- [v7 SGF22 source implementation on PR branch](../Multiprotocol/SGF22_nrf24l01.ino)
- Git blob SHA of source after CRLF -> LF normalization: `5e1abb555b7ddb5496117cb8d1acec28ea25ba76`.
- This file has been verified byte-identical (after line-ending normalization) to the source inside the original `Multiprotocol - Copy.zip` submitted with the exact tested binary.
- Additional matching archived configuration files (after line-ending normalization): `_MyConfig.h`, `_Config.h`, `Multiprotocol.h`, `Multi_Config.ino`, `Validate.h`, `Binary_Signature.ino`.
- Build settings in `_MyConfig.h`: `MULTI_AIR`; serial-only mode (`ENABLE_PPM` undefined); bootloader enabled through normal configuration.
- **An independent bit-identical rebuild of the binary has NOT been verified.**
- GitHub source and this file **do not by themselves host the firmware download**. A public GitHub binary/release asset should be made available separately before directing external testers to flash it.

## What v7 adds relative to earlier PR #1

1. F-35 flight packet sequence with separate lower counter and high-bit session state.
2. CH6-driven auxiliary behavior: packet byte 6 `0x20` plus session-sequence flag clearing on activation.
3. F-35 TX completion / RX turnaround scheduler during normal flight traffic.
4. Experimental one-byte enhanced-payload RX telemetry probe for stock-observed `E0/F0`. These bytes are **not a verified battery-voltage measurement**.
5. The existing hard-coded identity and bind behavior from the previously tested aircraft remain.

## What is **not** yet verified

- Safe motor/failsafe behavior following lost transmission.
- Cold power cycle reacquisition, connection recovery, or extended reliability on both airframes.
- Operation with other MultiModule RF frontends or alternative SAYZON F-35 revisions.
- General factory-pair ID, hop-set and flight-address derivation.
- Correct meaning and reliability of E0/F0 telemetry.

## Hardware and flashing caution

**For external developers and bench testers only.** This binary is for compatible STM32 MultiModule hardware, **not for flashing the Q X7 radio firmware**. Compatibility with alternate modules and module bootloader variants is unverified. Choose the correct flash method for your installed bootloader; see [STM32 compilation/flash instructions](Compiling_STM32.md) and [Flash from transmitter](Flash_from_Tx.md).

Run first tests with the **EDF/motor electrically disconnected**. Do not conduct an unrestricted flight on the strength of bind/control success alone. Preserve a known-good backup of the module firmware.

## Next engineering tests

1. Measure fail-safe motor command behavior and loss-of-link recovery under prop/motor-safe conditions.
2. Repeat cold receiver/transmitter power cycles on both airframes.
3. Capture independent replacement aircraft stock-TX A0/A1, ID, flight address and hop channels.
4. Validate E0/F0 status capture and semantics.
5. Independently build from the documented source/configuration and compare the resulting .bin hash to the known-working binary.

**License:** [GNU GPLv3](../LICENSE). Original MultiModule upstream: [pascallanger/DIY-Multiprotocol-TX-Module](https://github.com/pascallanger/DIY-Multiprotocol-TX-Module).
