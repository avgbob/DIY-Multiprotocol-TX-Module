# SAYZON 40 mm F-35 / ParkTen-style 2.4 GHz protocol reverse engineering

Status: **beta candidate / bench tested on one aircraft-transmitter pair**

This document records the reverse engineering and current experimental implementation of the 2.4 GHz protocol used by the SAYZON 40 mm F-35 EDF aircraft in `pascallanger/DIY-Multiprotocol-TX-Module`.

> Safety: all bind, ownership, recovery and arming tests should be performed with the EDF/motor electrically disconnected until the behavior is fully characterized.

## Hardware used

- SAYZON stock transmitter
- SAYZON 40 mm F-35 aircraft
- FrSky Taranis Q X7
- iRangeX IRX4 Plus 4-in-1 MultiModule
- MultiModule XN297 dump/debug firmware

The stock transmitter RF path appears to be:

```text
MCU -> 3-wire SPI -> XN297-family radio -> RFX2401C PA/LNA -> antenna
```

The packet radio is believed to be an XN297-family device, likely XN297LBW. The RFX2401C is the PA/LNA front end.

## Confirmed RF format

```text
radio family:       XN297
bitrate:            1 Mbps
scrambling:         enabled
payload format:     enhanced
address length:     5 bytes
bind RF channel:    28
bind address:       C7 95 3C BB A5
```

This places the F-35 very close to the existing MultiModule SGF22 family, but its bind frames, flight payload length and packet scheduling are different enough to keep it as its own SGF22 subtype.

## Stock bind/acquisition sequence

### A0 search

The stock transmitter begins with:

```text
A0 09 0B 81 00 08 00 92
```

on:

```text
channel 28
address C7 95 3C BB A5
period ~15.91 ms
```

With the aircraft powered off, the stock transmitter can remain in A0 indefinitely. Therefore A0 -> A1 is not a simple transmitter timer.

### Receiver participation / ACK-like event

The current MultiModule experiment sends the same A0 frame and, immediately after TX completion, turns the NRF around to RX for a short window.

A valid zero-length enhanced receive event in that window causes the implementation to advance immediately to A1.

Bench result:

- the aircraft binds substantially faster than the old fixed ~5.1 s approximation;
- on a fresh model setup, the control surfaces performed the bind/init movement once rather than the earlier two-stage-feeling behavior;
- behavior is consistent with the receiver acknowledging A0 and the transmitter using that event to leave search.

This is strong evidence for an ACK-like receiver-participation event, but it has only been tested on one aircraft/transmitter pair. The exact native XN297 ACK semantics have not been independently captured on the stock transmitter SPI bus.

### A1 confirmation

After receiver participation, the transmitter sends:

```text
A1 00 00 00 00 08 00 92
```

on the same bind channel/address for approximately 200-225 ms.

Enhanced-payload PID continuity across A0 -> A1 indicates that A1 is transmitter-originated.

### Current acquisition state machine

The beta-candidate implementation now behaves as:

```text
startup / Bind
    |
    v
A0 SEARCH
ch 28 / C7 95 3C BB A5
A0 09 0B 81 00 08 00 92
    |
    | receiver participation detected
    v
A1 CONFIRM
A1 00 00 00 00 08 00 92
~14 packets / ~223 ms
    |
    v
FLIGHT
```

A0 is no longer aged out by an arbitrary five-second timer. The transmitter remains in search until the receiver participates.

## Why startup must acquire instead of immediately transmitting flight packets

Two-transmitter testing exposed a receiver ownership/reacquisition state.

Observed behavior:

1. **Q X7 controls aircraft; stock TX is powered on**
   - stock TX remains blinking/searching;
   - it does not immediately take control;
   - when the Q X7 is switched off, the stock transmitter can acquire control very quickly.

2. **Stock TX controls aircraft; Q X7 blindly starts captured flight traffic**
   - control surfaces jitter/jump;
   - stock transmitter reacts/beeps;
   - interference stops when the Q X7 is removed.

The earlier direct-to-flight startup therefore differed from the stock state machine and could interfere with an already-owned receiver.

The current `sayzon-f35-stock-acquire` branch performs A0 acquisition on startup and only enters flight after receiver participation. Bench testing indicates this is the correct direction.

## Flight link

### Flight address

```text
55 08 00 92 14
```

The fragment `08 00 92` also appears in A0 and A1. It is not yet known whether this value is globally fixed, model-specific or pair-specific.

### RF hop set

Confirmed flight channels:

```text
24, 39, 55, 71
```

Current implementation order:

```text
24 -> 55 -> 39 -> 71
```

This is the same four-channel set already present in the SGF22 hopping table.

### Packet sequence

Flight packets occur as same-channel pairs separated by approximately 3.96-3.98 ms.

Byte 0 follows:

```text
80 81
84 85
88 89
8C 8D
...
B8 B9
wrap -> 80 81
```

Implementation:

```c
first_copy = base;
second_copy = base | 0x01;

base += 0x04;
if (base > 0xB8)
    base = 0x80;
```

## Flight payload

Normal flight packets are 9 bytes:

```text
[0] [1] [2] [3] [4] [5]  [6] [7] [8]
SEQ THR RUD ELE AIL MODE  ??  ??  ??
```

Current mapping:

```text
byte 0  sequence
byte 1  throttle    01..FF
byte 2  rudder      01..7F..FF
byte 3  elevator    01..80..FF
byte 4  aileron     01..7F..FF
byte 5  mode        10 / 14 / 18
byte 6  normally 00
byte 7  normally 10
byte 8  normally 42
```

Q X7 mapping:

```text
CH1 Aileron
CH2 Elevator
CH3 Throttle
CH4 Rudder
CH5 3-position flight mode
```

CH5 mode values:

```text
-100% -> 0x10
   0% -> 0x14
+100% -> 0x18
```

No application-level checksum has been identified; XN297 enhanced-payload CRC provides link-layer integrity.

Special byte-6 values such as `0x02` and `0x20`, and occasional unusual byte-0 values, remain unresolved.

## Motor arming is separate from RF acquisition

The stock transmitter performs a throttle-up / throttle-down gesture after the RF link is established.

Current testing indicates this is **not another bind stage** and probably is not a separate ARM channel.

Evidence:

- SGF22-family implementations expose many auxiliary flags, but no dedicated ARM channel for F22/F22S/J20/T28.
- The F-35 has full surface control after RF acquisition while the motor can still be in its safe/disarmed state.
- With the Q X7, only a small throttle excursion followed by returning to low appeared sufficient to arm the aircraft.

Current working hypothesis:

```text
RF acquisition:
A0 -> receiver participation -> A1 -> flight packets

motor safety:
throttle low -> throttle rises above an unknown threshold -> throttle low
-> motor armed
```

The exact throttle threshold and required timing are not yet characterized. The protocol firmware should **not automatically command an arming throttle excursion** until this is understood, because doing so could unexpectedly start the EDF.

## Comparison with existing SGF22/F22 implementation

Current upstream SGF22/F22 does not gate bind completion on an ACK or a specific aircraft message.

It uses:

```text
SGF22_BIND_COUNT = 50
```

and exits bind when that counter reaches zero.

If SGF22 telemetry is enabled, the implementation can receive a 3-byte aircraft packet containing transmitter-ID bytes and battery state, but that message is used for telemetry and does not cause `BIND_DONE`.

Therefore the existing F22 code is effectively a fixed-time working approximation. The F-35 implementation now models the experimentally observed receiver-driven A0 -> A1 transition instead.

## Development history

Important branches:

```text
sayzon-f35-experiment
    first working proof; forced timed A0/A1 on every init

sayzon-f35-normal-bind
    normal MultiModule Bind command; fixed timer

sayzon-f35-qx7-air
    normal-bind code plus AIR/serial profile for 128 KB STM32F103CB

sayzon-f35-bind-probe
    first generic RX probe; disrupted binding; superseded

sayzon-f35-ack-handshake
    immediate post-A0 zero-length enhanced receive probe;
    substantially faster successful bind

sayzon-f35-stock-acquire
    current beta candidate;
    performs receiver-driven acquisition on every startup so it does not
    blindly inject flight packets into an already-owned receiver
```

## What is considered proven on the current test pair

- XN297, 1 Mbps, scrambled enhanced framing
- bind address `C7 95 3C BB A5`
- bind channel 28
- A0 frame and ~15.91 ms cadence
- aircraft participation is required for the stock transmitter to leave A0
- A1 frame and ~200-225 ms confirmation phase
- immediate post-A0 receive detection makes the MultiModule bind much faster
- flight address `55 08 00 92 14`
- hop set `24,55,39,71`
- 9-byte flight payload and primary control mapping
- CH5 mode values `10/14/18`
- startup acquisition is preferable to blind direct-to-flight traffic
- motor arming appears to be a post-link throttle-state operation

## What is not proven yet

- whether the zero-length receive event is exactly the stock XN297 hardware ACK mechanism or an equivalent zero-payload enhanced response
- whether `08 00 92` and the flight address are globally fixed or pair-specific
- whether all SAYZON/ParkTen F-35 units use the same hop/address values
- exact arming throttle threshold and timing
- meaning of byte-6 special values
- exact stock hop/pair scheduler details
- behavior on other MultiModule hardware / NRF clones
- long-range and in-flight recovery behavior

## Beta-test priorities

Additional owners are most useful if they can report:

1. whether `sayzon-f35-stock-acquire` binds their aircraft;
2. whether bind advances quickly after the aircraft is powered;
3. whether the model remains in A0/search when no compatible aircraft is available;
4. whether powering a second transmitter causes jitter or takeover;
5. whether a small low -> raised -> low throttle movement arms the motor;
6. whether their captured bind/flight address contains the same `08 00 92` identifier;
7. module type, radio type and exact aircraft branding/revision.

Please test with the EDF disconnected first.
