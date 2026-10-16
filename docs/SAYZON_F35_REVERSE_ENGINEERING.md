# SAYZON 40 mm F-35 / ParkTen-style 2.4 GHz protocol reverse engineering

Status: **experimental SGF22-family F-35 variant; binding and control validated on two separate SAYZON F-35 receivers using one captured stock-pair identity. Not yet a general F-35 release.**

**Development and review:** [avgbob/DIY-Multiprotocol-TX-Module PR #1](https://github.com/avgbob/DIY-Multiprotocol-TX-Module/pull/1) · [experimental implementation](https://github.com/avgbob/DIY-Multiprotocol-TX-Module/blob/sayzon-f35-beta-candidate/Multiprotocol/SGF22_nrf24l01.ino) · [protocol configuration](https://github.com/avgbob/DIY-Multiprotocol-TX-Module/blob/sayzon-f35-beta-candidate/Protocols_Details.md)

## October 15, 2026 — updated engineering status (read before historical notes)

The historical v7 source/protocol derivation below remains the baseline record. For Pascal's **current** technical review, use the [upstream review packet](SAYZON_F35_UPSTREAM_REVIEW_PACKET.md), the [raw archive index](captures/README.md), and the [current clean-driver PR #4](https://github.com/avgbob/DIY-Multiprotocol-TX-Module/pull/4). The [complete original 35-file capture ZIP](https://github.com/user-attachments/files/33471553/SAYZON_F35_REDISCOVERED_XN297DUMP_LOGS_20261002-03.zip) is **already attached** to [upstream issue #1196](https://github.com/pascallanger/DIY-Multiprotocol-TX-Module/issues/1196); six exact original logs are also committed in the fork.

**Distinct revisions and evidence:**

- **Original released v7:** one fixed stock-transmitter identity, binds/controls **two** separately purchased receiver airframes on the bench; exact 116,560-byte binary/sha below. Not flight-validated. A bit-identical source rebuild of **that exact v7 binary** is still not established.
- **PR #4 clean driver:** compiled in Arduino IDE on the actual STM32F103CB IRX4 Plus profile; exported 116,608-byte firmware (SHA-256 `75c16b30692b6c357ed89cc534754917cf2215ad6705ebdedd253fba47d64f33`). Operator reported seven motor-disconnected bench checks passed: boot, bind, primary controls, CH5 modes, CH6, receiver-only power-cycle reconnection, and cancelling/restarting bind. Host simulation found 476 transmitted packet events and callback timings equal to v7 for its modeled scenarios. **Different binary, not an identical rebuild of v7**, and still not flight/failsafe certified.
- **PR #4 remaining framework concern:** cancelling bind exactly while the A1 countdown equals 2 can be confused with global `End_Bind()` setting `bind_counter=2`. This narrow race remains for maintainer review, even though ordinary bench cancellation worked.
- **PR #2 independent trial:** candidate CH7/CH8/CH9 mode/aux bytes produced no observable EDF-disconnected response; operator reported inability to cancel Bind. The exact UI/module failure mode was not isolated. **Do not incorporate PR #2's binding logic** in the clean driver.
- **PR #5 independent trim experiment:** [draft, on PR #4](https://github.com/avgbob/DIY-Multiprotocol-TX-Module/pull/5), CH14-gated and host-tested only. It is *not* part of the reviewed v7 implementation and had no target build/receiver verification at documentation time.

**Corrections from re-reading the October 3 original control logs:**

- The mixed `20261003-135525` recording did include moving X-PILOT (operator confirmation). Its byte 5 `10 ↔ 00` transitions **do not establish an independent find-aircraft flag**. The isolated `20261003-141518` recording explicitly associates `18 / 10 / 14` with the three-position mode switch.
- Byte 6 `0x02` and `0x04` were **not found** in the recovered complete nine-byte stock-flight records. Neither the "di-di" beeper nor the stunt function is decoded by those values; both remain speculative. `0x20` is captured and is a **circling/loiter candidate**, not a flight-validated function.
- Bytes 7/8 **do change** in original trim recordings by steps including byte7 `1`/`0x20` and byte8 `1`/`4`. An unexplained byte8 high-bit `0x80` change remains. The actual mapping of axes/buttons to these packed fields has **not** been established. The original v7 sends neutral `10 42` without the changing stock trim fields.
- Original stock "find aircraft" testing did **not acoustically confirm** the aircraft's "di-di", and the EDF was disconnected. Absence of sound does not independently distinguish a missed/incorrect command from use of the motor windings as an acoustic device.

These newer results supersede **test-status claims and provisional feature interpretations** in earlier dated sections below. They do *not* invalidate the archived raw captures, two-airframe interoperability, or the outstanding question of correct XN297 ACK turnarounds and pair-ID derivation.

## v7 source and binary provenance (verified October 14, 2026)

The tester confirmed **`f35v7.bin` was the exact binary flashed to the IRX4 Plus** for both successful aircraft tests.

- Binary size: **116,560 bytes**.
- SHA-256: `3d1f1361cb77f2878216e52c040e12828ad88cb7046fcdeaf403e28a7a235479`.
- Embedded MultiModule version: **1.3.4.74**, STM32 firmware.
- Source: the uploaded `Multiprotocol - Copy.zip` contains the same binary and STM32 AIR, serial-only external-module build profile in `Multiprotocol/_MyConfig.h`.
- The ZIP's `Multiprotocol/SGF22_nrf24l01.ino` is now **exactly matched to the PR source** after normalizing CRLF to LF. Git blob SHA: `5e1abb555b7ddb5496117cb8d1acec28ea25ba76`.
- Other compared build files also match the ZIP after line-ending normalization: `_MyConfig.h`, `_Config.h`, `Multiprotocol.h`, `Multi_Config.ino`, `Validate.h`, and `Binary_Signature.ino`.
- **Build reproducibility has not been established:** no independent rebuild has yet been verified to produce the same `f35v7.bin`. These checks establish provenance of source file content and the user's flashed binary, not complete reproducibility of the binary.

[Review PR #1](https://github.com/avgbob/DIY-Multiprotocol-TX-Module/pull/1) · [v7 experimental release notes](SAYZON_F35_V7_RELEASE.md)

## October 14, 2026: independent receiver-airframe validation

**Reported bench result: PASS for binding and control operation on a replacement aircraft.** A replacement SAYZON 40 mm F-35 bound immediately to the same FrSky Taranis Q X7 running the existing modified MultiModule firmware, and the user verified that all controls operated as intended. The original F-35 had previously bound and operated with that firmware. The replacement did not require a newly derived identity, address, hop sequence, or firmware adjustment.

This establishes **observed interoperability with two separate physical receivers**, not a fully decoded general-purpose pairing scheme. Both tests used the same experimental hard-coded identity `08 00 92`, flight address `55 08 00 92 14`, and hops `24,55,39,71`. We have **not** captured the stock transmitter paired with the replacement aircraft, so we cannot conclude whether those values are factory-global, shared by some production units, or otherwise accepted by multiple receivers. A second independent *stock transmitter/aircraft RF capture* is still required to derive any general pair-ID/address/hop mapping.

**Not yet validated on the replacement receiver:** signal-loss failsafe, automatic reacquisition after signal loss, repeated power cycling, sustained RF reliability, actual on-air ACK equivalence, operation on other RF module hardware, or wider manufacturing revisions. Keep the PR in draft/engineering-test status; perform safety testing with the EDF/motor disconnected.



This document records the reverse engineering and current experimental MultiModule implementation for a SAYZON 40 mm F-35 EDF aircraft.

> Safety: perform bind, ownership, recovery, auxiliary-function and arming tests with the EDF/motor electrically disconnected until the behavior is fully characterized.

## Hardware used

- SAYZON stock transmitter
- SAYZON 40 mm F-35 aircraft
- FrSky Taranis Q X7
- iRangeX IRX4 Plus 4-in-1 MultiModule
- MultiModule XN297 dump/debug firmware

Observed stock-TX RF path:

```text
MCU -> 3-wire SPI -> XN297-family radio -> RFX2401C-class PA/LNA -> antenna
```

The exact radio/front-end markings should still be recorded from the PCB. This matters because the bind ACK turnaround depends on both the packet radio and the PA/LNA TXEN/RXEN timing.

## Family identification: this is an SGF22-family variant

Several items that initially looked F-35-specific are already present in upstream SGF22.

### Shared bind PHY/address

Confirmed F-35 bind PHY:

```text
XN297
1 Mbps
scrambled enhanced payload
5-byte address
bind address C7 95 3C BB A5
```

Upstream SGF22 uses the same bind address:

```c
XN297_SetTXAddr((uint8_t*)"\xC7\x95\x3C\xBB\xA5", 5);
```

The F-35 binds on RF channel 28. Upstream J20 and T28 also use channel 28.

### Hop set is an existing SGF22 row

The captured F-35 hop order is:

```text
24 -> 55 -> 39 -> 71
```

which is exactly SGF22 hop-table row 4:

```c
{ 0x18, 0x37, 0x27, 0x47 }  // 24, 55, 39, 71
```

Upstream SGF22 does **not** hard-code that row globally. It derives a row from its two-byte pair ID:

```c
uint16_t val = (rx_tx_addr[2] << 8) | rx_tx_addr[3];
if (rx_tx_addr[2] > (0xFF - rx_tx_addr[3]))
    val--;
val %= 5;
```

For the captured F-35, the two meaningful ID-looking bytes are `08` and `92`, separated by `00` in the bind/flight structures. Using either byte order as the SGF22-style two-byte ID lands on row 4:

```text
08,92 -> row 4
92,08 -> row 4
```

So the captured channels are **consistent** with those two bytes being the SGF22 family ID, but row 4 does not identify their order or prove the F-35 ID layout. Other nearby two-byte splits of the captured address do not land on row 4, which strengthens the consistency check but still does not provide a derivation.

The current F-35 code therefore keeps the tested pair hard-coded. A second stock pair is required to determine whether the pair ID selects the hop row, whether the hop row is fixed across F-35s, and how the flight address is derived.

## Stock acquisition state machine

The state machine itself is now clear:

```text
A0 search until receiver participation
        ->
A1 confirmation
        ->
flight
```

### A0

```text
channel: 28
address: C7 95 3C BB A5
period:  ~15.91 ms

A0 09 0B 81 00 08 00 92
```

With the aircraft powered off, the stock transmitter remains in A0 indefinitely. Therefore the transition is receiver-driven, not a bind timer.

The `09 0B 81 00` portion is still unread. It has been stable in the captures so far, but should be checked across repeated cold power cycles and another transmitter before being treated as a protocol constant. It is not the four-channel hop list.

### Empty enhanced ACK

The zero-length enhanced receive event is best understood as the XN297 enhanced-mode ACK, not an application-level aircraft message.

A native XN297 can perform the TX -> RX ACK turnaround in hardware. An empty ACK has zero application payload, so there is no ordinary receiver payload for the stock MCU to parse.

The MultiModule NRF24L01 path is emulating XN297 framing. The current F-35 implementation therefore reproduces the event by:

```text
send emulated XN297 A0
wait for TX completion
switch NRF/PA-LNA path to RX immediately
listen for a valid zero-length enhanced frame
advance to A1 on success
```

This worked on the iRangeX IRX4 Plus, but it is hardware-sensitive. The fragile part is the NRF plus external PA/LNA turnaround inside the ACK window. Success on one IRX4 Plus does not prove identical behavior on every NRF24L01 clone or on other RF backends.

Relevant native-radio concepts are auto-ack/retry configuration such as `EN_AA`, `SETUP_RETR`, `DYNPD/FEATURE`, and the TX result state (`TX_DS` versus retry failure). The current NRF emulation is not the same as simply enabling native NRF auto-ack because it is carrying an emulated XN297 on-air frame.

### A1

After participation:

```text
A1 00 00 00 00 08 00 92
```

is sent on the same bind channel/address for roughly 200-225 ms.

PID continuity across A0 -> A1 supports A1 being transmitter-originated.

### Candidate additional bind-channel message (`0x51`) — **hypothesis; not decoded**

The [unmodified stock-TX XN297Dump capture from October 2](captures/SAYZON_XN297_20261002_030108_bind_ACK_and_A1.txt#L575-L604) contains an **additional CRC-valid six-byte enhanced packet** on RF channel 28, using the standard bind address `C7 95 3C BB A5`, just after several A1 frames and before the first captured flight data:

```text
line 575: P(0)                            [A0 traffic resumes afterward]
line 576+: A0 09 0B 81 00 08 00 92     [continues after that P(0)]
line 597: P(0)
lines 598-601: A1 00 00 00 00 08 00 92
line 602: P(6)= 51 00 00 08 00 92    [enhanced PID 3, bind address]
line 603: P(0)                        [enhanced PID 0, bind address]
line 604: P(9)= 88 01 7F 80 7F 10 00 10 42  [flight address 55 08 00 92 14]
```

**Direct observations:** the six-byte `51 00 00 08 00 92` frame exists in the original log, contains the same `08 00 92` identity fragment as A0/A1, appears after captured A1 traffic, and precedes the transition to the flight address. A `P(0)` also appears **earlier** during A0, without an immediate transition to A1. The sniffer is passive, fixed-channel and misses some packets: its `us` times are intervals between received packets, not guaranteed intervals between every transmission.

**Possible interpretations, not yet established:**
- `0x51` could be a final bind/confirmation/end-of-acquisition command from the *stock transmitter*.
- Alternatively it could be a receiver-originated confirmation/status message, or unrelated to acquisition. The passive sniffer does **not** label transmitting direction; we must not classify it as an RX packet just because it follows A1.
- The earlier `P(0)` could be a genuine receiver reply missed or not acted on by the stock TX, a differently timed ACK, or some other zero-length enhanced event. Its observation **does not** prove the stock transmitter must advance on every decoded `P(0)`.

**Why this matters:** the present experimental v7 can control two physical airframes using an A0 → CRC-valid zero-length enhanced receive event → timed A1 → flight model, **without explicitly sending a `0x51` application packet**. Therefore this log does not show that a `0x51` step is required for either tested airframe, or justify changing the working firmware. It might matter for cold bind, receiver ownership/recovery, or different receiver revisions.

**Smallest validating experiment (EDF motor disconnected):** capture multiple independent cold binds with the stock transmitter, logging the entire A0/A1/`0x51`/flight transition; annotate original-TX vs receiver power events. Confirm whether `0x51` appears consistently and whether its enhanced PID sequence is continuous with A1. If possible capture TX/RX direction via controlled TX-only and RX-only sessions or a second synchronized sniffer; distinguish the first `P(0)` that did not end A0 from the later one. Compare the second stock transmitter's identity and bind sequence. Only then consider a code change.

**Status:** **OPEN HYPOTHESIS — NO FIRMWARE CHANGE**. Do not imply native-ACK causality, a fourth mandatory handshake phase, or proven message direction from this single fixed-channel trace.

## Pair ID and flight address

Captured flight address:

```text
55 08 00 92 14
```

A0/A1 contain:

```text
08 00 92
```

while `55` and `14` do not appear in either bind frame.

The most useful current interpretation is:

```text
08 ... 92  = pair identity carried during acquisition
55 / 14    = flight-address wrapper bytes, fixed or derived unknown
```

That makes this look like factory pairing rather than a bind exchange that teaches a new flight address. The aircraft appears to recognize its existing identity while listening to A0.

With only one stock pair, we cannot yet determine whether:

- `08/92` changes per pair;
- the inserted `00` is part of the ID or a fixed field;
- `55` and `14` are fixed wrappers;
- `55` and/or `14` are derived from the pair ID.

This is the main blocker to a general release.

## Flight payload

Captured F-35 flight packets are 9 bytes:

```text
[0] [1] [2] [3] [4] [5]  [6] [7] [8]
SEQ THR RUD ELE AIL MODE  AUX FT? FT?
```

Current observations:

```text
byte 0  sequence / sequence flags
byte 1  throttle       floor 01
byte 2  rudder         center 7F
byte 3  elevator       center 80
byte 4  aileron        center 7F
byte 5  mode           10 / 14 / 18
byte 6  auxiliary flags; normally 00
byte 7  normally 10
byte 8  normally 42
```

### Fine-tune sentinel bytes

Upstream SGF22 sends:

```c
packet[10] = 0x42; // no fine tune
packet[11] = 0x10; // no fine tune
```

The F-35 has the same two sentinel values in the opposite order at the end of its shorter packet:

```text
byte 7 = 10
byte 8 = 42
```

They should be treated as inherited no-fine-tune sentinels unless a stock-transmitter trim sweep shows them changing.

### Auxiliary byte

F-35 byte 6 is the likely SGF22-family auxiliary flag byte.

Two observed values already overlap named upstream SGF22 bits:

```text
0x02  SGF22_FX922_FLAG_BALANCE
0x20  SGF22_T28_RTH_SET
```

That does **not** prove those two functions have the same user-facing meaning on the F-35; it does show that treating the bits as random unknown data is no longer useful.

The next test is a systematic stock-button sweep while watching byte 6. Candidate family bits to watch include the usual SGF22 toy-function masks such as `0x04`, `0x08`, `0x10`, `0x20`, `0x40`, and `0x80`.

### Mode byte is a real F-35 divergence

Captured three-position values:

```text
-100% -> 0x10
   0% -> 0x14
+100% -> 0x18
```

Unlike upstream SGF22, which folds flight mode into flag bits, the F-35 has a dedicated mode byte.

The values advance in steps of four, so when auxiliary buttons are swept, byte 5 should also be checked for additional low/high bits rather than assuming every feature lives only in byte 6.

## Stick encoding needs exact endpoint/center work

Captured neutral/floor values are asymmetric:

```text
throttle floor  0x01
rudder center   0x7F
elevator center 0x80
aileron center  0x7F
```

The current implementation already avoids transmitting throttle/axis `0x00` by replacing zero with `0x01`, but upstream `convert_channel_8b()` naturally centers at `0x80`.

A proper stock stick sweep should establish:

- exact minimum/maximum for each axis;
- exact neutral byte at physical center;
- whether `0x00` is reserved/failsafe-like;
- whether rudder/aileron intentionally center at `0x7F` rather than being capture noise or trim.

## Sequence and pair scheduler remain open

Captured first-byte pairs:

```text
80 81
84 85
88 89
...
B8 B9
```

The +4 progression is family-like. Upstream SGF22 contains a long-standing note that its sequence can appear as `0x02..0x7A` and sometimes with `0x80` ORed in.

Therefore the F-35's `0x80` should not yet be treated as a unique protocol flag. The occasional unusual/unflagged byte-0 values may be ordinary family sequence behavior or resynchronization.

What still needs a clean timing capture:

- gap between the two packets in a same-channel pair;
- gap between adjacent pairs;
- exact hop transition relative to the pair;
- whether the second member is an application-level second packet or a hardware retry;
- whether enhanced PID is reused or incremented;
- sequence-to-hop phase relationship.

This is more important than further decoding of already-recognized family constants.

## Motor arming remains unproven

The aircraft has surface control after RF acquisition while the motor can remain in its safe/disarmed state.

On the Q X7, a small throttle excursion followed by returning to low appeared to arm the aircraft. That suggests receiver-side throttle-state arming, but it is not yet proven.

The decisive stock capture is:

```text
after A1 / flight:
hold low
perform stock throttle-up -> throttle-down gesture
log byte 1 and byte 6 continuously
```

Interpretation:

- if only byte 1 follows the stick, arming is receiver-side throttle-state logic;
- if byte 6 pulses or another field changes independently of the stick, there is a protocol arm command.

No automatic throttle excursion should be added to firmware until this is settled.

## Receiver ownership / failsafe window

Two-transmitter testing showed that the receiver maintains an active-link ownership state.

Observed qualitatively:

- when Q X7 owns the aircraft, stock TX remains searching;
- when Q X7 disappears, stock TX acquires quickly;
- blindly injecting the captured flight stream while stock TX owns the receiver causes jitter/interference.

The **ownership hold time has not been measured**. That time is effectively part of the receiver's link-loss/failsafe behavior.

Needed measurement:

```text
establish owner
remove owner at a known timestamp
measure time until waiting transmitter receives participation / gains control
```

This will show whether recovery requires a fresh A0 acquisition after link loss and how long the receiver protects its current owner.

## RX/telemetry path after A1

The **exact two-aircraft-tested v7 source snapshot** now contains an experimental receive phase during normal flight: it waits for TX completion, turns the RF module to RX, and probes for a one-byte enhanced payload on the flight address. The raw `0xE0/0xF0` status observed in stock captures is forwarded as raw A1/BATT telemetry when `SGF22_HUB_TELEMETRY` is enabled. **Do not interpret this as calibrated voltage or a verified low-battery flag.** The `0x10` bit polarity and meaning are unresolved, and reliable telemetry reception on the Q X7 has not been separately demonstrated.

The v7 source also provides CH6 as a switch setting byte 6, bit `0x20`; on first activation it clears the sequence high bit `0x80` for the rest of that session. The exact user-facing stock feature of this CH6 function remains subject to verification.

The stock transmitter/receiver behavior may differ from upstream SGF22, which can receive a three-byte enhanced packet `[id0,id1,battery-state]` on the bind address. Follow-up captures should verify when, whether, and how the F-35 emits `E0/F0`.

## Current code limitation

The present beta branch still contains:

```c
static const uint8_t SGF22_F35_flight_addr[5] =
    { 0x55, 0x08, 0x00, 0x92, 0x14 };

static const uint8_t SGF22_F35_hops[4] =
    { 0x18, 0x37, 0x27, 0x47 };
```

Those values are intentionally preserved from the one captured stock transmitter/aircraft pair, even though that configuration has now successfully controlled two different receiver airframes.

It should **not** be generalized by guessing an ID/address derivation from one sample. A second stock transmitter/aircraft capture is required first.

## Highest-value next capture

The highest-value external beta result is a second stock pair.

For another F-35, capture:

1. A0 and A1;
2. the flight address;
3. all four flight RF channels;
4. a few normal flight packets.

The key comparison is the identity:

```text
first pair: 08 00 92
second pair: ?
```

Interpret the second pair as follows:

```text
ID changes + hop row changes to another existing SGF22 row
    -> pair ID likely selects the hop row
    -> derive F-35 hops from the pair ID

ID changes + hop row remains 24,55,39,71
    -> hop table is likely fixed for this F-35 variant
    -> only the address/identity path needs to become pair-specific

ID stays the same across independent aircraft
    -> factory-global identity/addressing becomes more plausible
```

The second pair is what breaks the current ambiguity. One sample must not be used to invent the ID mapping.

## Current conclusion

The handshake is no longer the principal mystery. The important result is:

```text
A0 until receiver participation
-> empty enhanced ACK
-> A1
-> flight
```

The remaining work is mainly to map this F-35 packet layout onto already-known SGF22 family concepts and to determine which fields are pair-specific.

The existing v7 build is suitable for a clearly labeled **experimental development preview**, not a general flight-ready release. A broader beta should wait for at least one additional stock transmitter/aircraft capture, explicit failsafe and recovery testing, and a reproducible build because the current code still bakes one captured identity, one flight address and the corresponding SGF22 row-4 hop set into firmware.
