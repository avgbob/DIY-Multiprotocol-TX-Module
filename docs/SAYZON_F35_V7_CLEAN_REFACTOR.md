# SAYZON F-35 clean v7 protocol-driver refactor

**Status: DRAFT / NOT FLIGHT-VALIDATED / NO NEW RF CAPTURES REQUIRED.**
Source: [F-35 inside SGF22](../Multiprotocol/SGF22_nrf24l01.ino). Base: `sayzon-f35-beta-candidate` whose F-35 source matches the tested `sayzon-f35-v7-exp.1` tag after line-ending normalization. This branch **does not** incorporate trial PRs #2 or #3. Existing `f35v7.bin` and release tag are untouched.

## Scope

Retain MultiModule's existing `PROTO_SGF22` and subtype `SGF22_F35=5`; the EdgeTX protocol registration, physical radio configuration and other SGF22-family subtypes are not altered. Give F-35 its own driver internals within the existing `SGF22_nrf24l01.ino`:

- `F35_init`: initializes RF through the existing SGF22 setup, then the dedicated F-35 acquisition lifecycle.
- `F35_build_bind_packet` / `F35_send_bind`: separate literal A0/A1 payloads from TX operations.
- `F35_build_control_packet`: pure 9-byte packet construction independent of RF. This retains v7 throttle/axis conversion, `CH5` mode values `10/14/18`, `CH6` auxiliary `20`, session flag handling and fixed `10 42` trim neutral.
- `F35_advance_control_sequence` / `F35_send_control`: preserve paired sequence numbering and the fixed captured hopping sequence.
- `F35_process_status`: encapsulates v7 one-byte response handling. **The payloads D0/E0/F0 are NOT decoded battery voltages.** Legacy raw `v_lipo1` behavior is deliberately unchanged in this behavior-preserving refactor and must not be advertised as calibrated voltage.
- `F35_callback`: explicit `F35_ACQUIRE_SEND_A0`, `F35_ACQUIRE_WAIT_TX`, `F35_ACQUIRE_LISTEN`, `F35_CONFIRM_A1`, `F35_FLIGHT_SEND`, `F35_FLIGHT_WAIT_TX`, `F35_FLIGHT_LISTEN`, and `F35_CANCELLED` states.
- Existing public `SGF22_init` and `SGF22_callback` dispatch to these helpers for F35 only; non-F35 code path is preserved.

## What changes functionally

One deliberate safety/framework difference: the old A0/A1 division inferred protocol phase from a shared `bind_counter` sentinel; `End_Bind()` can set that counter to `2` before any valid ACK, erroneously advancing to A1. The new state machine enters A1 **only after** a CRC-valid zero-length enhanced packet is received in the acquisition RX window. If bind is manually cancelled, it stops transmitting and remains in `F35_CANCELLED` until the protocol is reinitialized. In A1, a changed bind counter normally triggers cancellation instead of accelerating to flight.

**Edge case still requiring framework review:** if the user cancels during the very short A1 window precisely when its remaining count is already `2`, the legacy `End_Bind()` assignment of `2` is indistinguishable from the expected value. Prefer an explicit framework cancellation event in a subsequent review rather than adding global protocol-specific hacks prematurely. This is not claimed fully resolved.

**Preserved:** fixed F-35 factory pair identity `08 00 92`, flight address `55 08 00 92 14`, hop row `24,55,39,71`, A0 search and A1 frames, all control packet bytes and v7 callback period/turnaround values. No `0x51` frame, new modes, extra auxiliary bits, generated IDs, alternate trim packing or battery-value assumptions are enabled.

## Tests performed (exact committed source)

The compiled host test used **the precise source committed in this branch**, Git blob SHA `b447e77c2ebc65b8df5269a5f2c69252dbf86ef7`. Original v7 source Git blob SHA `5e1abb555b7ddb5496117cb8d1acec28ea25ba76`. Both were built as standalone C++11 test harnesses with mocked XN297 radio operations (`g++ -Wall -Wextra -Werror`, and unused-function/variable warnings disabled for Arduino-specific fragments).

**PASS:** 1,400 ordinary callback iterations and **476 TX events** matched original v7 **exactly** for emitted packet bytes, RF channel, callback return delays, mode and bind flags; test includes two missed ACKs, one valid simulated zero-length ACK, full A1, three CH5 modes, CH6 activation/deactivation and its persistent sequence flag, all four control-axis endpoint/zero cases, and hop/sequence rollover.

**PASS:** separate tests exercised manual bind cancellation during A0, cancellation mid-A1, rejection of invalid/nonzero acknowledgment length, and absence of further TX packets after cancellation. Because these are mocked tests, they do not validate real receiver ACK timing, failsafe or flight behavior.

**Not run:** actual STM32/IRX4 Plus target firmware build (ARM cross-compiler/toolchain unavailable in the working environment); physical motor-disconnected bench regression; real failsafe, telemetry semantics or RF reacquisition tests. There is **no new flashable binary** and the v7 release remains the fallback.

## Required acceptance before adoption

1. Build STM32 MULTI_AIR serial-only external IRX4 Plus firmware, inspect size and compile warnings; **do not use this source with the wrong module profile**.
2. With EDF electrically disconnected, verify stock pair acquisition, surface direction, mode switch, CH6 session behavior and receiver power cycles.
3. Cancel bind while receiver is powered off and confirm no A1/flight transition or transmitter output; reinit and verify normal bind.
4. Preserve the known-working `f35v7.bin` and avoid flight tests until RF recovery, failsafe and return-to-neutral behavior are verified.

**Related evidence:** [captured stock F-35 sessions](captures/README.md), [review packet](SAYZON_F35_UPSTREAM_REVIEW_PACKET.md), [reverse-engineering detail](SAYZON_F35_REVERSE_ENGINEERING.md).