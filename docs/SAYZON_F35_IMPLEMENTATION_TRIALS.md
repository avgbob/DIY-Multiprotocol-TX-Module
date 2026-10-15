# SAYZON/Park10 F-35 — code-only implementation trials

**Branch:** `sayzon-f35-implementation-trials`, based on the known-working `sayzon-f35-beta-candidate`. The `sayzon-f35-v7-exp.1` tagged source, the exact `f35v7.bin` and the draft PR #1 are **not modified**.

**Objective:** test hypotheses derived from existing original captures, the aircraft's Chinese manual, and other *decoded* MultiModule implementations. **No new XN297 captures are required for this trial plan.** Actual RX behavior must still be checked with the existing aircraft.

**Risk:** experimental RF control. **Disconnect the EDF/motor electrically during every first-pass test.** Do not fly these trial builds. Restoring `f35v7.bin` is the defined rollback.

## Code switches and expected test behavior

| Control | Default behavior | Trial change in this branch | Evidence & confidence |
| --- | --- | --- | --- |
| CH5 (3-position) | Sends `0x10/0x14/0x18` in flight byte 5 | Unchanged, unless CH7 overrides | **High**, stock October 3 isolated mode sweep |
| CH6 (two-position switch) | Sends byte 6 `0x20` while high and permanently clears seq bit 7 on first activation | Unchanged, comment now calls it circling *candidate*, rather than confirmed RTH | **Medium-high**, stock long-press behavior, original manual `盘旋/退出`, decoded ParkTen/P10 left long-press `0x20` |
| CH7 (trial) | Ignored by v7 | While high, force byte 5 `0x00`; releasing restores CH5 value | **Medium** that `0x00` is a valid sustained state; **low** confidence about its physical meaning. It was seen for sustained packet runs in mixed capture |
| CH8 (trial) | Ignored by v7 | While high, OR byte 6 with `0x02` | **Low**, originally hypothesized find-aircraft; earlier stock trim-button activity did **not** confirm this value |
| CH9 (trial) | Ignored by v7 | While high, OR byte 6 with `0x04` | **Low**, proposed one-key stunt using the decoded ParkTen/P10 stunt flag as a *different-protocol analogy* |
| A0/A1 | ACK-controlled binding | Internal ACK-seen boolean prevents global `End_Bind()` forcing counter to 2 and prematurely starting A1 | **High static-code confidence**, yet no actual bind-cancel hardware test |
| RX status | `D0/E0/F0` raw byte was placed in `v_lipo1` telemetry field | Retains raw debug/status but **does not advertise undecoded bytes as battery voltage** | **High** that the voltage interpretation was unjustified; the code does **not** decode actual battery status |
| `0x51` frame | No explicit transmission | Compile-time `SGF22_F35_TRIAL_POST_A1_51 = 0` — **OFF** | **Low**, observed once on bind channel after A1; no sender/necessity proof |

### Bench test order (no new sniffer recordings)

1. **Baseline regression**: all trial channels LOW, CH5 middle, CH6 LOW. Verify A0 search → bind → all control surfaces, no motor needed. Compare with unchanged v7 binary behavior. Failure means the new build/branch is not suitable.
2. **Bind-cancel edge**: with receiver powered OFF, request then cancel Bind on the Q X7; firmware should **not** advance to flight by treating the global counter change as a real ACK. Power RX normally and confirm acquisition works. Do **not** use motor as an indicator.
3. **CH7 candidate mode `00`**: apply only after stable link, with motor disconnected, monitor surface/stabilizer response, compare CH5 3-position behavior, return CH7 LOW.
4. **CH8 candidate `02`**: try separately; motor disconnected means absence of audible beeps does not disprove a beeper possibly using motor windings. Do not assert successful find-aircraft solely from a radio UI change.
5. **CH9 candidate `04`**: try separately with aircraft secured and motor disconnected; if stunt is a state-machine command, expect that a subsequent stick input may be required. Do not attempt an actual roll with motor connected.
6. **CH6 circling**: can test bit behavior via radio switch and host-code tests; physical orbit capability is **not** meaningfully validated on a motor-disconnected bench. Do not fly just to test this stage.

**Decision rule:** leave the known-working v7 as the main release. Promote a candidate feature only when an observable aircraft response establishes its intended effect. A build succeeding or a host-side packet test passing does not establish receiver compatibility.

## Reproducible host tests performed

Tested the **exact Git blob** of `Multiprotocol/SGF22_nrf24l01.ino` on this branch against a C++11 host stub of the actual F-35 send and callback functions. The host source was regenerated from the local original v7 snapshot and its Git blob SHA verified byte-for-byte against the branch. The tests compile with `g++ -std=c++11 -Wall -Wextra -Werror`.

PASS: first A0, End_Bind counter guard (no phantom A1), real CRC-valid simulated zero-length ACK starting A1, A1 completion, original v7 neutral 9-byte flight frame with switches low, CH7 mode `0x00`, CH8 `0x02` and CH9 `0x04`, CH6 `0x20` and persistent session-bit behavior. These are **software-only simulations** with a mocked XN297 transceiver; they do **not** demonstrate radio timing, transmitter interoperability, target MCU compilation, or receiver operation.

## Closest decoded source evidence

- [SGF22 original protocol](https://github.com/pascallanger/DIY-Multiprotocol-TX-Module/blob/master/Multiprotocol/SGF22_nrf24l01.ino): XN297 enhanced/CRC, bind address, ID-derived hopping rows, trim constants, 3-position CH5 logic and intermittent telemetry.
- [Original SGF22 decoding issue #944](https://github.com/pascallanger/DIY-Multiprotocol-TX-Module/issues/944): explains full hop-row calculation including overflow correction; warns ID-specific channels matter.
- [SGF22 telemetry issue #1097](https://github.com/pascallanger/DIY-Multiprotocol-TX-Module/issues/1097): actual ParkTen F22S and KF700 J20 battery-status replies, not equivalent to F-35's one-byte replies.
- [XK2/P10 ParkTen](https://github.com/pascallanger/DIY-Multiprotocol-TX-Module/blob/master/Multiprotocol/XK2_ccnrf.ino): strong analogy for left-button long-press `0x20` circling and `0x04` stunt, **different over-air packet layout**.
- [V761](https://github.com/pascallanger/DIY-Multiprotocol-TX-Module/blob/master/Multiprotocol/V761_nrf24l01.ino): gyro assistance/calibration, beeper, return and trim field concepts, different protocol.
- [XERALL](https://github.com/pascallanger/DIY-Multiprotocol-TX-Module/blob/master/Multiprotocol/XERALL_nrf24l01.ino): packed trim fields and button short/long-press semantics; no direct numeric mapping proof to F-35.
- [Original unmodified F-35 captures](captures/README.md), [detailed notes](SAYZON_F35_REVERSE_ENGINEERING.md).

## Deliberately NOT implemented yet

- `0x51` as mandatory handshake, because it appears in one passive fixed-channel capture and v7 already binds two receivers without transmitting it. An optional compile-time code path exists for later A/B testing only.
- Guessed trim-channel-to-byte7/byte8 mapping. The original recordings show steps of `1`, `0x20`, and `4` but do not uniquely identify which physical trim control writes each field. Sending incorrectly packed trim bits might create unexpected surface offsets.
- Universal generated ID, fresh receiver pairing or hop derivation: **we have only one captured stock-transmitter identity**.
- Battery-voltage polarity, calibration or automated arming: this requires verified receiver responses, not a borrowed semantic label.

**Goal:** each trial changes one distinguishable variable and has a known-good rollback. No claim of flight readiness.
