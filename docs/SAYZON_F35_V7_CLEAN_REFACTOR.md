# SAYZON F-35 clean v7 protocol-driver refactor

**Status as of October 15, 2026: DRAFT / STM32 BUILD AND MOTOR-DISCONNECTED BENCH TEST PASSED / NOT FLIGHT-VALIDATED.**
Source: [F-35 inside SGF22](../Multiprotocol/SGF22_nrf24l01.ino). Base: `sayzon-f35-beta-candidate` whose F-35 source matches the tested `sayzon-f35-v7-exp.1` tag after line-ending normalization. This branch **does not** incorporate trial PRs #2 or #3. Existing `f35v7.bin` and release tag are untouched.

## Verified target firmware and receiver bench work — October 15, 2026

This section **supersedes** the historical "Not run" statement further down; source and compiled firmware are different revisions of **PR #4**, not a rebuilt copy of the exact released `f35v7.bin`.

- **Exact PR #4 F-35 source Git blob:** `b447e77c2ebc65b8df5269a5f2c69252dbf86ef7` (same source used in the host simulation; no source change to this PR's driver since that verification).
- **Actual hardware target build:** Windows Arduino IDE Export Compiled Binary, Multi X-in-1 STM32F103CB (128KB) with bootloader, no debug; STM32 AIR/serial external module profile. Exported `multi-stm-1.3.4.74.bin`, **116,608 bytes** including 24-byte MultiModule footer; SHA-256 **`75c16b30692b6c357ed89cc534754917cf2215ad6705ebdedd253fba47d64f33`**. Reported sketch size **116,584 bytes**; available application flash **120,808 bytes**, leaving **4,224 sketch bytes**, or **4,200 firmware-file bytes including footer**. Original **116,560-byte v7** remains separately preserved.
- **Reported motor-disconnected receiver test:** **seven checks passed** on at least one F-35 receiver: Q X7 recognizes and boots IRX4 Plus module; receiver binding; aileron/elevator/rudder response; three CH5 modes; CH6 auxiliary control; receiver-only power-cycle reconnection; bind cancellation and restart. The second physical receiver has been tested with **original v7**, but **PR #4 is not claimed tested on both**.
- **What is *not* proven:** real-time RF captures of the PR #4 output, correct native-XN297 ACK semantics/CRC, residual `bind_counter==2` cancel collision in the A1 window, timed packet-loss recovery, receiver failsafe, reliable actual voltage/status interpretation, motor arming, flight behavior and general pairing-ID derivation.
- **Build-tool caveat:** a modern Arduino CLI v1.5.1 build ran the GCC compile/link but failed at the older MultiModule `do_version` post-build step (`exit status 0xffffffff`). **Arduino IDE export succeeded**; this isn't a demonstrated source compilation error, nor a bit-identical v7 reproduction.
- **Reproducibility limitation:** the C++11 host tests described below were conducted with a mocked XN297 layer. Their **standalone harness is not committed in this repository**; this report provides methodology and expected values but not a runnable test command checked into version control.

The [current review packet](https://github.com/avgbob/DIY-Multiprotocol-TX-Module/blob/sayzon-f35-beta-candidate/docs/SAYZON_F35_UPSTREAM_REVIEW_PACKET.md) summarizes this branch for Pascal. Experimental PR #2/PR #5 results are deliberately **not imported** into this source.

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

**Historical status at the time the host tests were written (superseded October 15):** target STM32 build and physical bench testing had not yet been performed. **Subsequent results are documented at the top of this file:** a 116,608-byte Arduino IDE export was built and flashed, seven receiver bench checks were reported PASS, and its SHA-256 was recorded. **Still not done:** flight, timed RF-loss/failsafe or telemetry validation. The PR #4 binary is a distinct local bench artifact, **not the exact v7 release or a newly published GitHub firmware release**; original v7 remains the rollback.

## Required acceptance before adoption

1. **Completed (reported):** STM32 MULTI_AIR serial-only external IRX4 Plus export using the correct Arduino IDE board/profile; recorded binary size and hash. Do **not** use this file with the wrong module profile.
2. **Completed (reported):** EDF-disconnected baseline on at least one receiver — startup, bind, AIL/ELE/RUD, CH5 modes, CH6 and receiver power-cycle reconnection. This was not an RF timing trace.
3. **Partly completed:** operator successfully cancelled/restarted bind during bench test. Confirming **on-air absence of A1/flight packets after cancellation** and attempting the precise `A1 remaining == 2` race remain open; simulated no-RF tests are not physical evidence.
4. **Open:** reproduce measured signal-loss behavior, receiver failsafe/return-to-neutral, telemetry semantics, PA/LNA ACK timing validity and any generalization across stock transmitters or other MultiModule hardware. No motor-connected test or flight approval.
5. Preserve released `f35v7.bin` as known-good fallback; keep PR #4 draft until the remaining framework and safety questions are reviewed.

**Related evidence:** [captured stock F-35 sessions](captures/README.md), [review packet](SAYZON_F35_UPSTREAM_REVIEW_PACKET.md), [reverse-engineering detail](SAYZON_F35_REVERSE_ENGINEERING.md).