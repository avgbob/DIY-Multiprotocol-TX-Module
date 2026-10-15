# SAYZON F-35: guarded packed-trim hypothesis

**Status:** EXPERIMENTAL SOURCE ONLY. Derived from already archived October 3 XN297Dump captures. **No new RF captures required.** This is not a proven trim decoder and not a flight-ready release.

**Branch:** `sayzon-f35-packed-trim-trial` extends `sayzon-f35-implementation-trials` and preserves all known-working v7 functionality when CH14 is LOW. The tested `f35v7.bin` is unchanged.

## Why test these four fields?

Actual original-stock TX packet-tail changes during control sweeps:

- Byte 7 neutral `0x10`; changes by `±0x01` (seen `0F/11`) and `±0x20` (seen `F0`, `F1`, `CF` etc.).
- Byte 8 neutral `0x42`; changes by `±0x01` (seen `41`) and `±0x04` (seen `3E`, `B2/B6/BA/BE`). Bit `0x80` also changes in separate recordings; its independent meaning is not settled.
- The upstream SGF22 stock firmware uses tail `42 10` as no-fine-tune constants (reverse order to F-35), while XK2/P10, V761 and XERALL examples demonstrate packed trim/mode fields; **none proves the F-35 bit allocation**.

**Hypothesis:** four independent digital trim fields, with step sizes split between bytes 7 and 8. This branch tests *whether the receiver responds* to these isolated increments, not yet which physical trim button corresponds to each one.

## CH controls — enable gate and one step only

**CH14** is the experiment enable gate. Keep CH14 LOW except while deliberately testing. CH10–CH13 are **field probes A–D**, not labelled rudder/elevator/aileron/throttle.

| Channel | LOW (-100%) | Neutral (0%) | HIGH (+100%) | Affected field |
| --- | --- | --- | --- | --- |
| CH10 | `byte7=0F` | `byte7=10` | `byte7=11` | Candidate trim A (step 1) |
| CH11 | `byte7=F0` | `byte7=10` | `byte7=30` | Candidate trim B (step 0x20) |
| CH12 | `byte8=41` | `byte8=42` | `byte8=43` | Candidate trim C (step 1) |
| CH13 | `byte8=3E` | `byte8=42` | `byte8=46` | Candidate trim D (step 4) |

Values above assume **only one probe active at a time**, and CH14 HIGH. At CH14 LOW, both bytes are always `10 42` regardless of CH10–CH13.

## Physical A/B bench check — no further sniffing

1. Keep **EDF disconnected**. Secure the plane with power to receiver/servos only; use the Q X7/IRX4 Plus and the experimental firmware branch.
2. First verify baseline with CH14 LOW, all channels CH7–CH13 neutral/LOW as applicable, and normal stick and X-PILOT operation.
3. Configure CH14 as a **two-position enable** switch; temporarily assign a **three-position test switch** to only one of CH10, CH11, CH12, CH13 at a time; the other three channels must be **center (0%)**, not -100%.
4. Move probe one notch from center to +100%, then to -100%, then back center while watching each control surface. Identify whether any surface changes position and if it recovers at neutral. Repeat for A, B, C, D. Do not exceed one step or run all channels at once on the first pass.
5. Return CH14 LOW, verify all surfaces and control modes return to baseline. If any unexpected motion appears, stop and revert to the original `f35v7.bin`.

**Pass condition for a field:** a repeatable, reversible and correct-signed neutral offset in one surface, **without** unintended throttle, arming or gyro changes. **Fail/uncertain:** no motion, multiple surfaces, persistent offset, or change only during a special flight-control state. Note outcomes per probe in ordinary words; no new RF captures are necessary.

**Important:** a stabilized elevon aircraft can move more than one physical surface in response to one logical control. Differentiate a trim offset from gyro stabilization effects by keeping the aircraft motionless and using exactly the same assistance mode throughout. Receiver surface behavior still requires a physical test.

## Host-side code tests

- The exact Git blob SHA of the modified `Multiprotocol/SGF22_nrf24l01.ino` was reproduced from the archived local snapshot: `9522adb2659d8f1b081aec293a3f2b52a388913d`.
- Compiled the extracted actual F-35 send/callback path with `g++ -std=c++11 -Wall -Wextra -Werror` using a mocked XN297 layer.
- PASS: A0/ACK/A1 state, prevention of A1 after forced counter cancellation, baseline v7 packet, candidate mode and auxiliary bytes, session flag, CH14 disabled gate, **all eight signed trim probe results**.

**Not tested:** target IRX4 Plus STM32 compilation, actual RF exchange/timing, physical trim effects, flight, battery telemetry. **Do not replace the user's tested v7 firmware as the reference.**

## Evidence
- [Clean October 3 isolated mode-switch capture](captures/SAYZON_XN297_20261003_141518_control_trim_sweep.txt)
- [Original 35-file capture archive](https://github.com/user-attachments/files/33471553/SAYZON_F35_REDISCOVERED_XN297DUMP_LOGS_20261002-03.zip)
- [SGF22 source](https://github.com/pascallanger/DIY-Multiprotocol-TX-Module/blob/master/Multiprotocol/SGF22_nrf24l01.ino)
- [P10 packed trim decoder](https://github.com/pascallanger/DIY-Multiprotocol-TX-Module/blob/master/Multiprotocol/XK2_ccnrf.ino)
- [XERALL packed trim decoder](https://github.com/pascallanger/DIY-Multiprotocol-TX-Module/blob/master/Multiprotocol/XERALL_nrf24l01.ino)
