# SAYZON 40 mm F-35 / SGF22 — upstream review packet

**Prepared for** [upstream issue #1196](https://github.com/pascallanger/DIY-Multiprotocol-TX-Module/issues/1196).  
**Reviewer request:** Please sanity-check the SGF22-family interpretation and identify what would need to change for an upstreamable implementation. I can modify code and run controlled physical tests, but I do not claim expert knowledge of XN297 internals, RF timing, or protocol-ID derivation.

This is a **review/evidence summary**, not a general compatibility claim or a request that maintainers reverse engineer from scratch.

## 1. Entry points — read in this order

1. **[F-35 SGF22 implementation (reconciled v7 source)](../Multiprotocol/SGF22_nrf24l01.ino)**; specifically [A0 ACK state machine](https://github.com/avgbob/DIY-Multiprotocol-TX-Module/blob/sayzon-f35-v7-exp.1/Multiprotocol/SGF22_nrf24l01.ino#L370-L454), [flight TX/RX scheduling](https://github.com/avgbob/DIY-Multiprotocol-TX-Module/blob/sayzon-f35-v7-exp.1/Multiprotocol/SGF22_nrf24l01.ino#L457-L519), and [initialization](https://github.com/avgbob/DIY-Multiprotocol-TX-Module/blob/sayzon-f35-v7-exp.1/Multiprotocol/SGF22_nrf24l01.ino#L610-L634).
2. [Development PR #1 in the fork](https://github.com/avgbob/DIY-Multiprotocol-TX-Module/pull/1) (**this is *not* an upstream PR**).
3. [Detailed interpretation / prior experiments](SAYZON_F35_REVERSE_ENGINEERING.md).
4. [Exact user-tested firmware `f35v7.bin` and notes](https://github.com/avgbob/DIY-Multiprotocol-TX-Module/releases/tag/sayzon-f35-v7-exp.1).

### Newly recovered raw XN297Dump evidence

Earlier notes incorrectly described the full serial dumps as unavailable. **They were uploaded during the original October 2–3 decoding work and have now been recovered.** There are **35 original uploads totaling 10,132,887 bytes**, including **31 files with roughly 110,764 parsed `RX ... Enhanced pid` records** across repeated sessions and captures. These are not necessarily unique packets or independent capture sessions. Four additional files are scan/config output or notes rather than enhanced RX-record streams.

**Six exact uploaded files are now public**, their Git blob identities checked against the bytes of the original files:

- [A0 search / zero-length enhanced packet / A1 transition: 20261002-030108](captures/SAYZON_XN297_20261002_030108_bind_ACK_and_A1.txt#L590-L603). This shows multiple `P(0)` records near A0/A1, but **does not conclusively prove their origin or that each event is a native hardware ACK**.
- [Clean A0→A1 transition: 20261002-034930](captures/SAYZON_XN297_20261002_034930_A0_to_A1.txt#L750-L762). Observe ~15.9 ms intervals between several A0/A1 frames.
- [A0-only: 20261002-035844](captures/SAYZON_XN297_20261002_035844_A0_only.txt). Over 1000 enhanced A0 records, no captured A1 in this file; **the aircraft's power state for this particular capture is not independently established by the log**.
- [Flight packets: 20261002-025214](captures/SAYZON_XN297_20261002_025214_flight_CH24.txt). Captures `C=24` control traffic, some ~3.97 ms intervals, and a [one-byte `F0` packet](captures/SAYZON_XN297_20261002_025214_flight_CH24.txt#L611).
- [Flight E0/F0 observations: 20261002-152421](captures/SAYZON_XN297_20261002_152421_flight_E0_F0_probe.txt#L360-L378). Full log includes ~8,676 enhanced RX records, with 32 one-byte `E0` and one `F0` records on C=24. **No low-battery/voltage meaning is asserted**.
- [Control/trim capture: 20261003-141518](captures/SAYZON_XN297_20261003_141518_control_trim_sweep.txt).

The [full capture index](captures/README.md) lists all 35 original file names, byte sizes and counts, with a checksum-verified complete ZIP prepared for posting as an upstream issue attachment. The archive is preserved **outside GitHub** until attached; the six linked raw files above are already public.

## 2. Hardware, capture method, firmware provenance

| Item | Status / value |
| --- | --- |
| Test aircraft | SAYZON 40 mm F-35 EDF; original and separately purchased replacement (two distinct physical receiver airframes) |
| Radio | FrSky Taranis Q X7 |
| Module flashed and used for control | **iRangeX IRX4 Plus external STM32 MultiModule**; the Q X7 radio firmware was **not** flashed with this file |
| Capture method | MultiModule's own **XN297Dump/protocol-dump firmware flashed onto the IRX4 Plus**, used to sniff original SAYZON stock-transmitter transmissions; afterward the IRX4 Plus was flashed with the experimental SGF22-F35 implementation |
| Earlier dump setup recorded | `XN297DP`, `1Mbps`, receiver `05`, RF channel set manually as required; captured observations summarized below |
| Stock TX RF hardware | Suspected XN297-family transceiver with RFX2401C-class PA/LNA; exact markings/revision still to be confirmed |
| Firmware used on both airframes | `f35v7.bin`; 116,560 bytes; MultiModule signature version 1.3.4.74 |
| SHA-256 | `3d1f1361cb77f2878216e52c040e12828ad88cb7046fcdeaf403e28a7a235479` |
| Source provenance | Tester confirms exact flashed binary; a second copy in their recovered source ZIP was byte-identical. Recovered SGF22 .ino now matches the repository file after line-ending normalization, Git blob `5e1abb555b7ddb5496117cb8d1acec28ea25ba76` |
| Build profile | [`_MyConfig.h`](../Multiprotocol/_MyConfig.h): `MULTI_AIR`, `ENABLE_PPM` disabled / serial control (Q X7 external module), STM32 MultiModule profile |
| Reproducibility | **Not established:** no independent bit-identical rebuild from recovered source has been verified |

There is **one captured stock transmitter identity**, not two. The second airframe's **stock transmitter has not been independently RF-captured**. Separately purchased aircraft + same fixed ID working is evidence of interoperability, *not* proof that the code derives or negotiates IDs universally.

## 3. RF captures / behavior that can presently be supplied

The following are **values derived from recovered XN297Dump serial captures and our lab notes**. The public raw files linked above enable reviewers to confirm individual packets and tool-reported `us` intervals. **Original timestamped serial capture files have now been recovered from the experimenter's October 2–3 uploads and published in this fork:** see the [capture inventory and six byte-for-byte original logs](captures/README.md). The complete 35-file, 10.1 MB original-upload corpus is also preserved in a checksum-indexed ZIP for the experimenter to attach to upstream issue #1196. The selected original records include A0, zero-length `P(0)` packets, A1, flight packets and one-byte `E0/F0` payloads. This supports our *observations*, not all our interpretations. Captures are primarily on fixed channels C=24 or C=28; we still lack comprehensive time-correlated multi-channel recordings. Please treat absolute RF timing/hops as observations requiring confirmation, not as archived independent trace evidence.

| Field | Captured / working value | Qualification |
| --- | --- | --- |
| PHY | XN297-family, scrambled enhanced payload, 1 Mbps, 5-byte address | Interpreted from captured framing; stock TX chip marking still unverified |
| Bind RF channel | 28 | Fixed-channel observation |
| Bind address | `C7 95 3C BB A5` | Also used upstream SGF22 |
| Stock A0 | `A0 09 0B 81 00 08 00 92` | Period approximately **15.91 ms** |
| Stock A1 | `A1 00 00 00 00 08 00 92` | Approximately **12–14 packets / 200–225 ms** after receiver participation |
| TX-only behavior | With aircraft **off**, stock TX continued A0 rather than entering A1 | Evidence transition likely depends on receiver participation; **does not alone decode ACK** |
| Receiver response | Zero-application-byte **enhanced ACK is our interpretation**; emulated RX check `XN297_ReadEnhancedPayload(packet_in, 0) == 0` works on tested IRX4 Plus | Exact on-air receiver ACK and PCF/PID handling not independently proven |
| Flight address | `55 08 00 92 14` | Hard-coded from one stock pair |
| Flight hop sequence | 24 → 55 → 39 → 71 (`18 37 27 47` hex) | Matches SGF22 upstream hop table row 4; general derivation unknown |
| Flight packet | 9 bytes: `SEQ THR RUD ELE AIL MODE AUX 10 42` | Axes present on surface controls; presumed fine-tune sentinel interpretation remains tentative |
| Mode byte | `10` / `14` / `18` (hex) | Physical mode labels not independently verified |
| Channel floors/centers | throttle floor `01`; rudder/aileron center `7F`; elevator center `80` | Exact end-point and trim behavior not completely characterized |
| Flight transmission interval | Approximately **3.96–3.98 ms** between captured packets at the relevant observed sequence; implementation uses 3970 µs | Same-channel pair / inter-pair timing and precise hop relationship not fully resolved |
| Byte 0 sequence | `80 81`, `84 85`, ... through `B8 B9` observed during a startup/session | v7 source also implements a lower sequence count and a high-bit flag that CH6 activation clears; full state interpretation still uncertain |
| CH6/aux | v7 sends byte 6 `0x20` when CH6 active; high sequence bit clears on first activation | Raw-capture-inspired implementation, exact user-facing function still unproven |
| Enhanced 1-byte response | `E0/F0` reported in original stock capture interpretation | v7 *probes* for 1-byte replies; reliable reception and battery semantics NOT bench-verified |

## 4. Hardware observations and tests — avoid overstating

| Test/claim | Current status |
| --- | --- |
| First physical F-35 responds to experimental firmware | **Reported PASS** |
| Separately purchased second physical F-35 binds immediately, all controls work with **same binary** | **Reported PASS** |
| Module bind mode is used with the new F35 subtype | **Reported working** |
| Aircraft-off stock TX remains sending A0 | **Observed in prior original-TX capture tests** |
| Competing stock TX owns receiver, our module waits in A0 | **Qualitatively observed**, not timestamped / not verified on second receiver |
| Independent stock-transmitter ID, hop or flight-address capture from second kit | **Not performed / unavailable** |
| Fresh factory/virgin bind proven distinct from reuse of stored fixed ID | **Not proven** |
| Explicit Bind-cancel while in F35 A0, then check for false A1 | **Not bench-tested** |
| Receiver-only power-cycle recovery and timed reacquisition | **Not systematically tested** |
| True RF loss/recovery without restart, receiver failsafe, motor behavior | **Not tested to release standard** |
| Original fixed-channel timestamps / packet records | **Recovered and linked:** [raw XN297Dump files](captures/README.md), including A0/A1 and one-byte E0/F0 responses. **Still not established:** complete multichannel synchronized timing, reliable classification of ghosts, definitive physical origin/ACK causality |
| Actual voltage / low-battery meaning of `E0` versus `F0` | **Unknown** |
| Bit-identical recompilation of `f35v7.bin` | **Not demonstrated** |

## 5. The specific decisions needing experienced review

### A. TX completion, ACK handling, and RF turnaround — first priority

[F35 ACK code](https://github.com/avgbob/DIY-Multiprotocol-TX-Module/blob/sayzon-f35-v7-exp.1/Multiprotocol/SGF22_nrf24l01.ino#L389-L428): send A0, poll `XN297_IsPacketSent()` every 25 µs for up to 600 µs, call **`XN297_SetTxRxMode(RX_EN)` directly** (no intermediate `TXRX_OFF`), listen for 700 µs, accept `XN297_ReadEnhancedPayload(packet_in, 0) == 0`. Bind period target 15,910 µs.

The `XN297_EMU.ino` wrapper **can** change from TX to RX directly, but should it here? In [related issue #1145](https://github.com/pascallanger/DIY-Multiprotocol-TX-Module/issues/1145#issuecomment-5147108627), Pascal suggested an OFF transition for one TX/RX test case. Are our zero-width RX configuration, PCF/CRC validation, TX-complete poll, and 700 µs timing correct for this ACK? Is there a better native MultiModule idiom or known side effect from the PA/LNA path? This is a **request for review**, not an assertion of a defect.

[Flight RX probe](https://github.com/avgbob/DIY-Multiprotocol-TX-Module/blob/sayzon-f35-v7-exp.1/Multiprotocol/SGF22_nrf24l01.ino#L457-L519) uses the same technique, but with width 1 and `E0/F0`; status has not been validated as actual low-battery telemetry.

### B. Bind-cancel / framework integration — actionable code-level concern

[F35 initialization](https://github.com/avgbob/DIY-Multiprotocol-TX-Module/blob/sayzon-f35-v7-exp.1/Multiprotocol/SGF22_nrf24l01.ino#L610-L634) initializes **`bind_counter = 0xFFFF`** to remain in A0 search until ACK. In [global `End_Bind()`](https://github.com/avgbob/DIY-Multiprotocol-TX-Module/blob/sayzon-f35-v7-exp.1/Multiprotocol/Multiprotocol.ino#L766-L776), protocols outside the exception list have their bind counter reduced to **2**. In the [F35 callback](https://github.com/avgbob/DIY-Multiprotocol-TX-Module/blob/sayzon-f35-v7-exp.1/Multiprotocol/SGF22_nrf24l01.ino#L370-L454), **2** falls into A1 transmission instead of ACK-search logic. Consequently an **`End_Bind()` call during A0 would advance it without an actual ACK**. That follows from current source, but whether/how Q X7 triggers it in actual use is **not yet tested**. What is the right framework-compliant way to guard/cancel an indefinite ACK search?

### C. ID and upstream architecture

[Captured pair values](https://github.com/avgbob/DIY-Multiprotocol-TX-Module/blob/sayzon-f35-v7-exp.1/Multiprotocol/SGF22_nrf24l01.ino#L113-L114) are **fixed**. The SGF22 row-index calculation appears consistent with `08`/`92` selecting row 4, but **one original stock TX is insufficient** to prove derivation or packet field semantics. Would Pascal prefer (i) an expressly fixed-ID engineering subtype while learning the format, (ii) more ID-capture work before any upstream PR, or (iii) a different protocol structure? I can run focused tests and revise the implementation in whichever direction is appropriate.

### D. Evidence missing for a broader release

I can capture and provide additional experiments if directed. My current limits are **not knowing enough to trust my XN297 ACK/CRC interpretation**, lack of an independently captured second stock transmitter identity, **no complete, time-correlated multi-channel acquisition trace or independently captured second stock transmitter**, and no systematic motor-safe loss-of-link or telemetry measurements. Rather than guessing at these details, I would appreciate guidance on **which single experiment would be decisive first**.

## 6. Safety and scope

**Experimental pre-release only.** The binary targets the **external STM32 IRX4 Plus MultiModule**, *not* the Q X7 radio firmware or receiver. Never infer a broadly safe/flyable link from normal surface-control response. Disconnect the EDF/motor during initial capture, cancel, and recovery tests. Keep this fork PR draft until basic framework, failsafe, and ID behavior have been reviewed.

## 7. Minimal potential reviewer response

The most useful first response is simply whether **(A)** the empty enhanced ACK / TX→RX approach is plausible and **(B)** the bind cancellation guard should be changed before further experiments. A recommendation for one decisive next bench test is much more helpful than a general request for “more logs”; I'd be glad to generate precisely the required captures, including µs timing, from the actual aircraft.

**Supporting context:** [full reverse-engineering notes](SAYZON_F35_REVERSE_ENGINEERING.md) · [release notes, hash and hardware requirements](SAYZON_F35_V7_RELEASE.md) · [v7 exact source tag](https://github.com/avgbob/DIY-Multiprotocol-TX-Module/tree/sayzon-f35-v7-exp.1)
