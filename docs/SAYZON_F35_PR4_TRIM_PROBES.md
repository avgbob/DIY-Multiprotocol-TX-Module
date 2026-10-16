# SAYZON F-35 — CH14-gated packed-trim bench experiment on PR #4

**Status: EXPERIMENTAL / DO NOT MERGE OR FLY. EDF MOTOR ELECTRICALLY DISCONNECTED.**

This branch starts at `sayzon-f35-v7-clean-refactor` (PR #4), **not** the older PR #2 or #3 branches. The original tested `f35v7.bin`, PR #4 source and its bench-verified firmware are unchanged. This branch includes **no speculative CH7/CH8/CH9 feature commands** and preserves PR #4's bind/cancel state machine.

## What the original October 3, 2026 stock-transmitter captures prove

The original files are in [the recovered archive index](captures/README.md) and full [issue #1196 attachment](https://github.com/user-attachments/files/33471553/SAYZON_F35_REDISCOVERED_XN297DUMP_LOGS_20261002-03.zip).

- `20261003-135525`: 5,221 nine-byte flight packets; byte 6 always `00`. Byte 5 `10 <-> 00` occurred in a *mixed* recording in which the operator also moved X-PILOT; do not assign these transitions to find-aircraft. Byte 7 and byte 8 change persistently with trim activity: examples `10 -> 11`, `F1 -> F0`, `EF -> CF`, `42 -> 3E`, `3E -> 3D` and `39 -> 35`.
- `20261003-141227`: 3,463 packets. Tail transitions include reversible `byte8:42 -> 3E -> 42` and `byte7:10 -> 0F -> 10`. Other transitions change two bytes together, so direction/axis assignments are not established.
- `20261003-141518`: 4,042 packets; isolated X-PILOT sequence `byte5:18 -> 10 -> 14 -> 18 -> 14 -> 10`, with byte 6 always `00` and trim bytes constant at `10 C2`.
- `20261003-143145`: 4,798 packets; byte 8 `C2 -> 42 -> BE -> BA -> B6 -> B2 -> B6 -> BA -> BE` while byte 5/6/7 stay constant. **Bit 0x80 and packed boundaries remain undecoded.**

The observed increments suggest four packed trim subfields: byte7 +/-`0x01` and +/-`0x20`; byte8 +/-`0x01` and +/-`0x04`. The values are **candidate test perturbations only**, not a decoded physical trim mapping. The old constant v7 tail `10 42` is retained whenever the gate is low.

## Channel setup for Q X7 / IRX4 Plus

- Keep **motor/EDF electrically disconnected**. Power receiver and servos only; secure aircraft. Throttle fully low.
- Keep CH5 in its known middle X-PILOT position; CH6 low. CH7/CH8/CH9 are not part of this trial.
- **CH14: gate.** `-100`/LOW = original PR #4 byte tail `10 42`. `+100`/HIGH = experimental gate on; a **momentary** switch is preferable.
- **CH10–CH13: anonymous probes A, B, C, D.** Set **all four to center/0%** on the Q X7 Channel Monitor before enabling CH14. Use a three-position control assigned to exactly **one** probe at a time (`-100, 0, +100`). Others must be exactly `0`.
- Code rejects two or more simultaneously active probes and instead transmits `10 42`. This is an additional guard, **not a substitute for radio setup**.

| Probe | Channel | -100% test | 0% | +100% test | Affected byte |
|---|---|---|---|---|---|
| A | CH10 | `0F` | `10` | `11` | 7 |
| B | CH11 | `F0` | `10` | `30` | 7 |
| C | CH12 | `41` | `42` | `43` | 8 |
| D | CH13 | `3E` | `42` | `46` | 8 |

Table assumes all other probes are at 0%, CH14 is high, and unchanged byte is neutral. A high gate with all probes centered sends `10 42`.

## Motor-disconnected test sequence

1. **Build/verify new binary in Arduino IDE** using the same STM32 128KB/bootloader board and compiler settings that succeeded for PR #4. Verify flash usage and MultiModule footer signature before flashing. Preserve known-good PR #4 and `f35v7.bin`; do not overwrite either.
2. Confirm baseline startup, bind, all control surfaces, CH5 modes, receiver power-cycle reconnect, and bind cancellation with CH14 **LOW**. **Stop** if this regresses.
3. On Q X7 Channel Monitor ensure CH10–CH13 are at **0**, CH14 is at **-100**. Assign a 3-position switch to one of CH10–CH13; keep the others centered. Raise CH14 to +100 only for the specific test.
4. Test each probe A–D individually: `0 -> +100 -> 0 -> -100 -> 0`. Allow the receiver to settle at each position; hold aircraft motionless. Watch control surfaces and LED/gyro behavior. Record exactly which control surfaces move, if the offset persists, and whether returning to center restores them.
5. Return CH14 **LOW** between probes; confirm normal surface behavior. If anything abnormal occurs, stop, power down, and restore PR #4 firmware.
6. After all four probes, confirm the baseline again. No motor connected and no flight test with this branch.

The model has stabilized elevons, so a logical axis may operate multiple surfaces. A repeatable surface displacement is a **receiver response**, not by itself a complete assignment of the stock transmitter's trim direction. Do not call a test successful solely because the radio's channel monitor moved.

## Verification performed before publishing

- Used the exact PR #4 source blob `b447e77c2ebc65b8df5269a5f2c69252dbf86ef7` as the starting point. The change is limited to F-35 flight packet bytes 7/8 behind CH14, with CH5/CH6, binding, timings and hops unchanged.
- Host C++11 simulator: **PASS** — 1,400 callback steps and **476 transmitted frames** byte-for-byte identical to original PR #4 with gate low.
- Host C++11 cancellation tests: **PASS** — cancel before ACK, cancel in A1, no RF after cancel, ignore invalid ACK.
- Host C++11 four-field byte tests: **PASS** — 8 signed probes, 8 disabled-gate cases, and 3 gate/simultaneous-probe safety cases, **19 assertions** total.
- **Not done**: target STM32 compile for this new branch; actual IRX4 Plus flash/test; receiver trim validation; RF sniffer comparison of generated experimental packets; flight tests. No claims of receiver support or safe flight.

**Decision rule:** Keep PR #4 as the bench-verified baseline. Promote only a probe whose effect is repeatable, reversible, and agrees with the original stock capture. If findings conflict with the partial encoding hypothesis, revise the model instead of adding more guessed controls.
