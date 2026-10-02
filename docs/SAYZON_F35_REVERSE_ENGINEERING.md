# SAYZON 40 mm F-35 / ParkTen-style 2.4 GHz protocol reverse engineering

Status: **experimental, first successful control achieved with MultiModule**

This document records the path from stock-transmitter capture to a working experimental implementation for the SAYZON 40 mm F-35 EDF aircraft using `pascallanger/DIY-Multiprotocol-TX-Module`.

> Safety: all protocol bring-up and reconnect testing should be done with the EDF/motor disconnected until link behavior is characterized.

## Hardware used

- SAYZON stock transmitter
- SAYZON 40 mm F-35 aircraft
- Taranis Q X7
- iRangeX IRX4 Plus 4-in-1 MultiModule
- MultiModule XN297 dump/debug firmware
- Windows PC / PuTTY / Arduino IDE

The stock transmitter RF path was identified as approximately:

```text
MCU -> 3-wire SPI -> XN297-family radio -> RFX2401C PA/LNA -> antenna
```

The radio on the stock transmitter appears to be an XN297-family device (likely XN297LBW). The RFX2401C is the RF front end, not the packet radio.

## Capture setup

The IRX4 Plus was flashed with MultiModule's XN297 dump/debug firmware and used as the 2.4 GHz sniffer.

Useful XN297Dump settings:

```text
Protocol:   XN297DP
Subtype:    1Mbps
Receiver:   05
RF Channel: fixed as required
```

The protocol is:

- XN297
- 1 Mbps
- scrambled
- enhanced payload
- 5-byte address

## Bind discovery

### Bind channel and address

Repeated fixed-channel captures established:

```text
RF channel: 28
Address:    C7 95 3C BB A5
```

The normal bind-search frame is:

```text
A0 09 0B 81 00 08 00 92
```

It repeats about every:

```text
15.91 ms
```

The transmitter can remain in this A0 search state for a long time; it is not simply a short fixed timer in the stock transmitter.

### Confirmation burst

When the aircraft participates in binding, the stock system transitions to:

```text
A1 00 00 00 00 08 00 92
```

on the same bind channel/address.

Observed A1 duration is about:

```text
~200 ms
```

with roughly 12-14 packets depending on capture losses.

Enhanced-payload PID continuity was observed across the A0 -> A1 transition, strongly suggesting that A1 is transmitted by the stock transmitter.

### Important TX-only test

With the aircraft physically powered off, moving the stock transmitter sticks did **not** cause A0 to become A1.

The transmitter stayed in A0 indefinitely.

This established that aircraft participation is what causes the stock transmitter to advance, even though the exact aircraft-to-transmitter response packet was not decoded.

### Practical shortcut

For the first implementation, the receiver-side acknowledgement was deliberately ignored.

Instead, the experimental transmitter sends:

```text
A0 for ~5.09 seconds
A1 for ~0.22 seconds
then immediately enters flight traffic
```

This was sufficient to control the aircraft.

## Flight link

### Flight address

Captured flight address:

```text
55 08 00 92 14
```

The repeated identifier fragment is notable:

```text
08 00 92
```

It appears in the bind traffic and flight address, but it has not yet been established whether it is globally fixed, transmitter-specific, or pair-specific.

### RF channels

The true flight hop set is:

```text
24, 39, 55, 71
```

The existing SGF22 implementation contains the exact same set in the order:

```text
24 -> 55 -> 39 -> 71
```

That order was used for the experimental implementation.

### Packet timing

Captured same-channel packet pairs are separated by approximately:

```text
3.96-3.98 ms
```

The first byte follows a repeating paired sequence:

```text
80 81
84 85
88 89
8C 8D
...
B8 B9
wrap -> 80 81
```

Equivalent logic:

```c
first  = base;
second = base | 0x01;

base += 0x04;
if (base > 0xB8)
    base = 0x80;
```

## Flight payload mapping

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

No application-level checksum was apparent; XN297 enhanced payload CRC provides link-layer integrity.

Some special-state captures produced unusual byte-0 and byte-6 values. Those remain unresolved and should not be treated as normal flight encoding yet.

## Why SGF22 was chosen

The protocol has a striking match to the existing MultiModule SGF22 family:

- XN297
- 1 Mbps
- scrambled enhanced payload
- exact bind address `C7 95 3C BB A5`
- bind channel 28 is already used by SGF22 variants
- exact four-channel flight set
- SGF22 table already contains `24,55,39,71`

The F-35 differs enough in bind framing, payload length, and scheduling that it currently exists as an experimental SGF22 subtype rather than being assumed identical to an existing aircraft.

## Experimental MultiModule implementation

The test implementation added:

```text
Protocol: SGF22
Subtype:  F35
```

Experimental state machine:

```text
startup
  |
  v
BIND_A0
  ch 28
  addr C7 95 3C BB A5
  A0 09 0B 81 00 08 00 92
  ~5.09 s
  |
  v
BIND_A1
  ch 28
  same bind addr
  A1 00 00 00 00 08 00 92
  ~0.22 s
  |
  v
FLIGHT
  addr 55 08 00 92 14
  hops 24 -> 55 -> 39 -> 71
  9-byte control packets
```

The implementation was built against upstream commit:

```text
ee984c403d6aedf56a2ef66dd27705430c79b302
```

The full current MultiModule configuration overflowed the 128 KB STM32F103CB target, so a test build was made with only SGF22 enabled.

Result:

```text
Sketch uses 24472 bytes (20%) of program storage space.
Maximum is 120808 bytes.

Global variables use 3176 bytes (15%) of dynamic memory.
Maximum is 20480 bytes.
```

The resulting firmware successfully controlled the aircraft.

## Reconnect behavior: important distinction

The current ~5.3 second A0/A1 delay happens **only when the F35 protocol initializes**.

Once the transmitter reaches FLIGHT state, it remains there and continues transmitting flight packets. Therefore, ordinary temporary RF loss from going out of range does **not** automatically restart the 5-second bind sequence.

If both transmitter and receiver remain powered, the expected behavior is:

```text
temporary RF loss
    ->
TX continues flight packets
    ->
receiver comes back into range
    ->
receiver should reacquire normal flight traffic
```

That reacquisition time has not yet been measured.

The real concern is different:

1. **TX/module reboot or protocol restart in flight**
   - current experimental code would run the ~5.3 second startup sequence again before sending flight packets.

2. **Aircraft receiver brownout/reboot**
   - it is not yet known whether the receiver will immediately reacquire the remembered flight address/hop sequence or whether it requires A0/A1 again.

Both conditions need bench testing before treating the implementation as flight-ready.

## Recommended final behavior

The safer long-term state machine is likely:

```text
normal startup:
    go directly to remembered/fixed flight address and transmit controls

explicit Bind command:
    run A0 -> A1 -> flight

temporary RF loss:
    never stop flight traffic
    never automatically enter a multi-second bind delay
```

Whether normal startup can always skip bind depends on one remaining test: receiver cold-start/reboot recovery while the transmitter is already sending flight traffic.

## Next tests

### 1. Receiver reboot test

With EDF disconnected:

1. establish control
2. leave Q X7 and IRX4 powered and in F35 flight state
3. power aircraft off
4. wait ~10 seconds
5. power aircraft back on
6. do not press Bind
7. measure time until surfaces respond

If response is immediate/fast, the receiver remembers enough state to make direct-to-flight startup practical.

### 2. True RF-loss/reacquisition test

With EDF disconnected and both sides continuously powered:

1. enable MultiModule Low Power
2. establish control
3. create enough distance to lose control
4. move back toward the transmitter
5. measure how quickly controls return

This specifically tests out-of-range recovery without rebooting either side.

### 3. Explicit-bind-only firmware

If the two tests above succeed, change the implementation so F35 does not force autobind in `SGF22_init()`.

Normal protocol initialization should enter FLIGHT immediately, while the radio's explicit Bind request should start A0/A1.

## Normal MultiModule bind workflow experiment

MultiModule's serial core already provides the behavior we want:

- selecting a model/protocol normally initializes it with bind **not** in progress
- pressing the radio's **Bind** control sets the serial bind flag
- the core restarts the protocol with `BIND_IN_PROGRESS`
- protocols may use the global `bind_counter`
- clearing/canceling Bind causes the core `End_Bind()` path to shorten that counter so the protocol exits bind promptly

Several existing protocols follow this pattern rather than unconditionally forcing autobind. HiSky is a particularly clear example: its init routine sets its bind counter only when `IS_BIND_IN_PROGRESS`; otherwise it starts normal data mode. Q90C and Potensic similarly choose the bind RF address only when the framework says binding is active and then switch to the normal address when binding completes.

The F35 experimental implementation was therefore changed on branch `sayzon-f35-normal-bind` to use the same framework semantics:

```text
normal model selection / TX restart
    ->
F35 init sees BIND_DONE
    ->
set flight address 55 08 00 92 14
    ->
send flight traffic immediately

radio Bind command
    ->
MultiModule core sets BIND_IN_PROGRESS and restarts protocol
    ->
F35 sends A0 for 320 periods
    ->
F35 sends A1 for 14 periods
    ->
BIND_DONE
    ->
switch to flight address and normal traffic
```

The bind timing is still the experimentally proven fixed-time shortcut rather than the stock receiver-acknowledgement-driven transition. This branch tests **normal MultiModule user workflow**, not yet a fully decoded stock handshake.

## Validation result: normal Bind command works

The `sayzon-f35-normal-bind` branch has now been bench-tested with the radio's normal **Bind** control and the aircraft successfully binds.

This validates the MultiModule-side workflow:

```text
radio Bind command
    ->
MultiModule core sets BIND_IN_PROGRESS
    ->
F35 protocol is restarted in bind mode
    ->
A0 timed phase
    ->
A1 timed phase
    ->
BIND_DONE
    ->
normal flight traffic
```

This does **not** yet prove that the timed A0/A1 shortcut is universal across multiple aircraft/transmitter pairs, but it confirms that the protocol now works with the normal MultiModule user-facing bind flow rather than forcing bind on every startup.

The next validation target is normal startup/recovery **without pressing Bind**.

## Remaining unknowns

- exact aircraft -> transmitter event that causes the stock A0 -> A1 transition
- whether `08 00 92` is globally fixed or pair-specific
- whether `55 08 00 92 14` is fixed or generated
- exact stock hop scheduler/pair timing
- short/long press encoding for the second stock-transmitter button
- meaning of special byte-6 values such as `0x02` and `0x20`
- meaning of unusual pre-rebind byte-0 sequences
- physical labels/directions for all mode states

## Current conclusion

A working control link has been achieved without decoding the receiver-side bind acknowledgement.

That means the minimum useful protocol is already known:

```text
XN297 / 1 Mbps / enhanced / scrambled
bind:   ch28, C7 95 3C BB A5
A0:     A0 09 0B 81 00 08 00 92
A1:     A1 00 00 00 00 08 00 92
flight: 55 08 00 92 14
hops:   24 -> 55 -> 39 -> 71
payload: 9 bytes
```

The next priority is not more bind sniffing. It is proving fast recovery behavior and then separating normal startup from explicit binding.
