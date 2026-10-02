# SAYZON 40 mm F-35 / ParkTen-style 2.4 GHz protocol reverse engineering

Status: **single-pair engineering beta; not yet a general F-35 release**

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

Stock flight captures now show reverse enhanced traffic on the **flight address**
`55 08 00 92 14`:

```text
P(0)=
P(1)= E0
P(1)= F0
```

The one-byte `E0/F0` response is the current telemetry candidate.  The two values differ
only by bit `0x10`, but its meaning and polarity are **not yet assigned**.  A controlled
aircraft-battery test is required before calling it battery-low.

The experimental branch `sayzon-f35-telemetry-probe` adds a receive window after every
F-35 flight application packet without changing the captured 3.970 ms packet cadence:

```text
send flight packet
-> poll TX complete
-> direct TX -> RX turnaround
-> listen 700 us for a 1-byte enhanced response
-> restore TX timing for the remainder of the 3.970 ms slot
```

The probe configures the F-35 flight address for a one-byte receive payload, intentionally
ignoring zero-length ACKs so the `E0/F0` application byte can be isolated.  When a valid
one-byte response is decoded, its raw value is exposed through the existing SGF22 Hub
telemetry path as `v_lipo1` / A1-BATT.  This is a **raw probe value**, not a voltage
conversion.

Expected first validation:

```text
healthy / normal aircraft state -> observe raw E0 or F0 on the radio
change aircraft battery state   -> determine whether bit 0x10 changes with warning state
```

Only after that correlation should the probe be converted from raw status to a user-facing
battery-good / battery-low telemetry sensor.

## Current code limitation

The present beta branch still contains:

```c
static const uint8_t SGF22_F35_flight_addr[5] =
    { 0x55, 0x08, 0x00, 0x92, 0x14 };

static const uint8_t SGF22_F35_hops[4] =
    { 0x18, 0x37, 0x27, 0x47 };
```

That is intentionally left as the known-working single-pair implementation.

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

A formal beta release should wait for at least one additional stock pair because the current code bakes one captured identity, one flight address and the corresponding SGF22 row-4 hop set into firmware.
