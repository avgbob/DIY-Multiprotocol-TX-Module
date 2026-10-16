# SAYZON 40 mm F-35 / SGF22 — upstream review packet

**Prepared for** [upstream issue #1196](https://github.com/pascallanger/DIY-Multiprotocol-TX-Module/issues/1196).  
**Reviewer request:** Please sanity-check the SGF22-family interpretation and identify what would need to change for an upstreamable implementation. I can modify code and run controlled physical tests, but I do not claim expert knowledge of XN297 internals, RF timing, or protocol-ID derivation.

This is a **review/evidence summary**, not a general compatibility claim or a request that maintainers reverse engineer from scratch.

## October 15, 2026 — current reviewer status (supersedes older test-status rows below)

**Which revision is which:** [original v7 firmware and source](https://github.com/avgbob/DIY-Multiprotocol-TX-Module/releases/tag/sayzon-f35-v7-exp.1) remain the exact known-good **two-aircraft** baseline. [Draft PR #4](https://github.com/avgbob/DIY-Multiprotocol-TX-Module/pull/4) is a **separate, cleaner state-machine refactor**, compiled into a different binary and now **motor-disconnected bench-tested**. The PR #4 firmware is *not* bit-identical to released v7; host simulation compared its intended RF output and scheduling to v7. Neither version has been flight- or failsafe-certified.

| Item | Latest test evidence / remaining limits |
| --- | --- |
| Released v7 | `f35v7.bin`, **116,560 bytes**, SHA-256 `3d1f1361cb77f2878216e52c040e12828ad88cb7046fcdeaf403e28a7a235479`; binding and primary control observed on **two** separately purchased receiver airframes with the **same** hard-coded stock identity |
| PR #4 exact source | `SGF22_nrf24l01.ino` Git blob `b447e77c2ebc65b8df5269a5f2c69252dbf86ef7`; explicit acquisition/confirmation/flight/cancellation driver; **no guessed feature bytes** |
| PR #4 target build | Arduino IDE export on the STM32F103CB 128 KB IRX4 Plus profile; **116,608-byte** `multi-stm-1.3.4.74.bin`, SHA-256 `75c16b30692b6c357ed89cc534754917cf2215ad6705ebdedd253fba47d64f33`; **4,200 bytes application space remaining** (maximum 120,808) |
| PR #4 hardware bench | Operator reported **7/7 checks passed on at least one receiver**, EDF electrically disconnected: module boots/radio recognizes it, binding, AIL/ELE/RUD directions, CH5 three positions, CH6 auxiliary switching, receiver power-cycle reconnect, and bind cancel/restart. Receiver power-cycle recovery was *qualitative*, not timed RF-loss/failsafe verification |
| PR #4 host tests | 1,400 callback cycles and **476 TX events** matched original v7 source in a mock-XN297 trace (packet bytes, RF channels, callback delays and RF modes); synthetic cancellation tests passed. The executable host harness has **not been published as an independently runnable repository test** |
| Cancellation limitation | PR #4 guards ordinary bind cancellation, but `End_Bind()` setting `bind_counter=2` **exactly when an A1 confirmation countdown is already 2** remains an ambiguous, narrow race. Please review whether MultiModule should offer an explicit cancel event instead of overloading the counter |
| Independent trial PR #2 | Separate speculative CH7 `byte5=00`, CH8 `byte6|=02`, CH9 `byte6|=04` trial: **no observable response** during EDF-disconnected bench checks. Operator reported **“bind can't cancel”**; UI-versus-module cause was not independently isolated. PR #2 does **not** form part of PR #4 or released v7 |
| Independent trial PR #5 | [Draft gated trim experiment](https://github.com/avgbob/DIY-Multiprotocol-TX-Module/pull/5), based on PR #4, preserves normal payload when disabled. Host-only checks reported; **STM32 compilation and receiver tests not yet performed**. This is **not** the recommended upstream implementation |

**Best review entry point today:** inspect [PR #4 code](https://github.com/avgbob/DIY-Multiprotocol-TX-Module/pull/4) for framework lifecycle/ACK turnarounds; use exact v7 source and the raw stock-transmitter captures as the control. Priority questions remain (1) whether the software-emulated zero-length XN297 ACK and direct TX→RX handoff are electrically/protocol-correct, and (2) how to signal cancellation to a protocol whose A0 wait is intentionally indefinite. Repeated bench success **does not** resolve pairing generality, radio cloning, loss-of-link failsafe, or flight safety.

## 1. Entry points — read in this order

1. **[PR #4: the current clean F-35 driver](https://github.com/avgbob/DIY-Multiprotocol-TX-Module/pull/4)** — [design + hardware status](https://github.com/avgbob/DIY-Multiprotocol-TX-Module/blob/sayzon-f35-v7-clean-refactor/docs/SAYZON_F35_V7_CLEAN_REFACTOR.md), explicitly separated from unvalidated feature experiments.
2. **[Original exact v7 source](https://github.com/avgbob/DIY-Multiprotocol-TX-Module/blob/sayzon-f35-v7-exp.1/Multiprotocol/SGF22_nrf24l01.ino)** — [A0 ACK path](https://github.com/avgbob/DIY-Multiprotocol-TX-Module/blob/sayzon-f35-v7-exp.1/Multiprotocol/SGF22_nrf24l01.ino#L370-L454), [flight TX/RX scheduling](https://github.com/avgbob/DIY-Multiprotocol-TX-Module/blob/sayzon-f35-v7-exp.1/Multiprotocol/SGF22_nrf24l01.ino#L457-L519), and [initialization](https://github.com/avgbob/DIY-Multiprotocol-TX-Module/blob/sayzon-f35-v7-exp.1/Multiprotocol/SGF22_nrf24l01.ino#L610-L634). [Exact v7 `f35v7.bin` release](https://github.com/avgbob/DIY-Multiprotocol-TX-Module/releases/tag/sayzon-f35-v7-exp.1).
3. **[Unmodified stock transmitter evidence](captures/README.md)** — six inline raw logs plus the [complete 35-file ZIP already attached to issue #1196](https://github.com/user-attachments/files/33471553/SAYZON_F35_REDISCOVERED_XN297DUMP_LOGS_20261002-03.zip).
4. [Detailed reverse-engineering notes](SAYZON_F35_REVERSE_ENGINEERING.md) and [earlier fork PR #1](https://github.com/avgbob/DIY-Multiprotocol-TX-Module/pull/1) (not an upstream pull request). Experimental PRs #2/#3/#5 are separate, not the proposed core driver.

### Newly recovered raw XN297Dump evidence

Earlier notes incorrectly described the full serial dumps as unavailable. **They were uploaded during the original October 2–3 decoding work and have now been recovered.** There are **35 original uploads totaling 10,132,887 bytes**, including **31 files with roughly 110,764 parsed `RX ... Enhanced pid` records** across repeated sessions and captures. These are not necessarily unique packets or independent capture sessions. Four additional files are scan/config output or notes rather than enhanced RX-record streams.

**Six exact uploaded files are now public**, their Git blob identities checked against the bytes of the original files:

- [A0 search / zero-length enhanced packet / A1 transition: 20261002-030108](captures/SAYZON_XN297_20261002_030108_bind_ACK_and_A1.txt#L590-L603). This shows multiple `P(0)` records near A0/A1, but **does not conclusively prove their origin or that each event is a native hardware ACK**.
- [Clean A0→A1 transition: 20261002-034930](captures/SAYZON_XN297_20261002_034930_A0_to_A1.txt#L750-L762). Observe ~15.9 ms intervals between several A0/A1 frames.
- [A0-only: 20261002-035844](captures/SAYZON_XN297_20261002_035844_A0_only.txt). Over 1000 enhanced A0 records, no captured A1 in this file; **the aircraft's power state for this particular capture is not independently established by the log**.
- [Flight packets: 20261002-025214](captures/SAYZON_XN297_20261002_025214_flight_CH24.txt). Captures `C=24` control traffic, some ~3.97 ms intervals, and a [one-byte `F0` packet](captures/SAYZON_XN297_20261002_025214_flight_CH24.txt#L611).
- [Flight E0/F0 observations: 20261002-152421](captures/SAYZON_XN297_20261002_152421_flight_E0_F0_probe.txt#L360-L378). Full log includes ~8,676 enhanced RX records, with 32 one-byte `E0` and one `F0` records on C=24. **No low-battery/voltage meaning is asserted**.
- [Isolated mode-switch capture: 20261003-141518](captures/SAYZON_XN297_20261003_141518_control_trim_sweep.txt). The operator confirmed using the mode switch. Byte 5 changes `18 → 10 → 14 → 18 → 14 → 10` with other control/trim bytes steady; this is not evidence of trim-button encoding.

The [full capture index](captures/README.md) lists all 35 original file names, byte sizes and counts, with the [complete original ZIP now attached to upstream issue #1196](https://github.com/user-attachments/files/33471553/SAYZON_F35_REDISCOVERED_XN297DUMP_LOGS_20261002-03.zip); the six linked raw files above are also public in the fork.

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

The following are **values derived from recovered XN297Dump serial captures and our lab notes**. The public raw files linked above enable reviewers to confirm individual packets and tool-reported `us` intervals. **Original timestamped serial capture files have now been recovered from the experimenter's October 2–3 uploads and published in this fork:** see the [capture inventory and six byte-for-byte original logs](captures/README.md). The complete 35-file, 10.1 MB original-upload corpus is [already attached to upstream issue #1196](https://github.com/user-attachments/files/33471553/SAYZON_F35_REDISCOVERED_XN297DUMP_LOGS_20261002-03.zip) in a checksum-indexed ZIP. The selected original records include A0, zero-length `P(0)` packets, A1, flight packets and one-byte `E0/F0` payloads. This supports our *observations*, not all our interpretations. Captures are primarily on fixed channels C=24 or C=28; we still lack comprehensive time-correlated multi-channel recordings. Please treat absolute RF timing/hops as observations requiring confirmation, not as archived independent trace evidence.

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
| Flight packet | 9 bytes: `SEQ THR RUD ELE AIL MODE AUX TRIM? TRIM?` | The v7 implementation currently sends constant `10 42` as bytes 7–8, but stock capture sessions show changes in those fields during separate trim-related actions. Do **not** treat them as permanently constant or attribute byte-5 mode changes to trim |
| Mode byte | `10` / `14` / `18` (hex) | **Confirmed mode-switch-associated values** in the isolated October 3 capture. Mirrors existing SGF22's CH5 three-position *mode selection*, though SGF22 encodes mode flags in packet byte 8 while this F-35 variant uses byte 5. Physical mode labels still unverified |
| Channel floors/centers | throttle floor `01`; rudder/aileron center `7F`; elevator center `80` | Exact end-point and trim behavior not completely characterized |
| Flight transmission interval | Approximately **3.96–3.98 ms** between captured packets at the relevant observed sequence; implementation uses 3970 µs | Same-channel pair / inter-pair timing and precise hop relationship not fully resolved |
| Byte 0 sequence | `80 81`, `84 85`, ... through `B8 B9` observed during a startup/session | v7 source also implements a lower sequence count and a high-bit flag that CH6 activation clears; full state interpretation still uncertain |
| CH6/aux | v7 sends byte 6 `0x20` when CH6 active; high sequence bit clears on first activation | Raw-capture-inspired implementation, exact user-facing function still unproven |
| Enhanced 1-byte response | `E0/F0` reported in original stock capture interpretation | v7 *probes* for 1-byte replies; reliable reception and battery semantics NOT bench-verified |

### October 3 stock control bytes — source evidence, not claimed feature mappings

The [capture index](captures/README.md#october-3-control-and-trim-capture-audit--october-15-2026) distinguishes the raw logs from interpretation. In the original mixed `20261003-135525` stock-transmitter test (5,221 nine-byte records), byte 6 stayed `00` while byte 7/8 trim fields changed; the tester **also moved the X-PILOT switch**, so the observed byte 5 `10 ↔ 00` does **not** identify a find-aircraft command. A later isolated mode sweep `20261003-141518` confirms byte 5 values `18 → 10 → 14 → 18 → 14 → 10` with the other control/trim bytes steady. Two other October 3 recordings show reversible byte 7/8 increments around the trim fields; high bit `0x80` in byte 8 and physical trim-to-field assignment remain undecoded.

Across the latest complete, recognizable nine-byte flight-record audit of the recovered archive (100,352 records across 30 files, **not unique transmissions**), byte 6 contained `00` or `20`, with **no observed `02` or `04`**. Because much of the radio evidence is fixed-channel and the find-aircraft `di-di` function was **not acoustically confirmed with the EDF disconnected**, this means `02` is **unsupported**, not disproven as a possible extremely brief command. We **should not** call CH8 a proven beeper command or CH9 a proven stunt command. The original manual's long-press circling description makes `20` a plausible circling candidate, not flight-validated behavior.

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
| Explicit Bind-cancel while in F35 A0 | **Original v7: not documented as tested. PR #4: reported PASS** for cancelling and restarting bind (motor disconnected); narrow A1 counter=2 collision remains untested. **PR #2: “can't cancel” reported; mechanism not confirmed** |
| Receiver-only power-cycle recovery and timed reacquisition | **PR #4 qualitative reconnect: reported PASS**, but elapsed time, full radio-link loss and failsafe behavior **not systematically tested** |
| True RF loss/recovery without restart, receiver failsafe, motor behavior | **Not tested to release standard** |
| Original fixed-channel timestamps / packet records | **Recovered and linked:** [raw XN297Dump files](captures/README.md), including A0/A1 and one-byte E0/F0 responses. **Still not established:** complete multichannel synchronized timing, reliable classification of ghosts, definitive physical origin/ACK causality |
| Actual voltage / low-battery meaning of `E0` versus `F0` | **Unknown** |
| Bit-identical recompilation of `f35v7.bin` | **Not demonstrated** |

## 5. The specific decisions needing experienced review

### A. TX completion, ACK handling, and RF turnaround — first priority

[F35 ACK code](https://github.com/avgbob/DIY-Multiprotocol-TX-Module/blob/sayzon-f35-v7-exp.1/Multiprotocol/SGF22_nrf24l01.ino#L389-L428): send A0, poll `XN297_IsPacketSent()` every 25 µs for up to 600 µs, call **`XN297_SetTxRxMode(RX_EN)` directly** (no intermediate `TXRX_OFF`), listen for 700 µs, accept `XN297_ReadEnhancedPayload(packet_in, 0) == 0`. Bind period target 15,910 µs.

The `XN297_EMU.ino` wrapper **can** change from TX to RX directly, but should it here? In [related issue #1145](https://github.com/pascallanger/DIY-Multiprotocol-TX-Module/issues/1145#issuecomment-5147108627), Pascal suggested an OFF transition for one TX/RX test case. Are our zero-width RX configuration, PCF/CRC validation, TX-complete poll, and 700 µs timing correct for this ACK? Is there a better native MultiModule idiom or known side effect from the PA/LNA path? This is a **request for review**, not an assertion of a defect.

[Flight RX probe](https://github.com/avgbob/DIY-Multiprotocol-TX-Module/blob/sayzon-f35-v7-exp.1/Multiprotocol/SGF22_nrf24l01.ino#L457-L519) uses the same technique, but with width 1 and `E0/F0`; status has not been validated as actual low-battery telemetry.

**Candidate post-A1 `0x51` packet — evidence needing interpretation (not a known defect).** [Original stock capture, lines 575–604](captures/SAYZON_XN297_20261002_030108_bind_ACK_and_A1.txt#L575-L604) contains `P(0)` at line 575 followed by more A0; another `P(0)` at 597 followed by A1, then a six-byte `51 00 00 08 00 92` enhanced packet (line 602), another `P(0)`, then flight packets. The `0x51` frame shares `08 00 92` with A0/A1. **Unknown:** transmitting side, application meaning, whether packet is consistently present, and whether it participates in binding. Passive fixed-channel logs do not establish causality or TX/RX direction. The tested v7 works on two physical airframes without explicitly sending this packet. See [documented open hypothesis and bench verification plan](SAYZON_F35_REVERSE_ENGINEERING.md#candidate-additional-bind-channel-message-0x51--hypothesis-not-decoded). **Do not modify firmware or assert a mandatory extra phase on this evidence alone.**

### B. Bind-cancel / framework integration — actionable code-level concern

[F35 initialization](https://github.com/avgbob/DIY-Multiprotocol-TX-Module/blob/sayzon-f35-v7-exp.1/Multiprotocol/SGF22_nrf24l01.ino#L610-L634) initializes **`bind_counter = 0xFFFF`** to remain in A0 search until ACK. In [global `End_Bind()`](https://github.com/avgbob/DIY-Multiprotocol-TX-Module/blob/sayzon-f35-v7-exp.1/Multiprotocol/Multiprotocol.ino#L766-L776), protocols outside the exception list have their bind counter reduced to **2**. In the [F35 callback](https://github.com/avgbob/DIY-Multiprotocol-TX-Module/blob/sayzon-f35-v7-exp.1/Multiprotocol/SGF22_nrf24l01.ino#L370-L454), **2** falls into A1 transmission instead of ACK-search logic. Consequently an **`End_Bind()` call during A0 would advance original v7 without an actual ACK** (source-level issue, **not a demonstrated v7 hardware failure**). **PR #4 now separates cancellation into explicit local states; its ordinary bind-cancel/restart passed the reported physical bench test.** Its residual narrow case occurs if global cancellation sets `bind_counter=2` at exactly the same time its A1 counter legitimately equals 2. **PR #2 is not the fix:** it was reported unable to cancel, and the exact Q X7/module symptom wasn't isolated. What is the right framework-compliant way to signal cancellation of an indefinite ACK search without any counter collision?

### C. ID and upstream architecture

[Captured pair values](https://github.com/avgbob/DIY-Multiprotocol-TX-Module/blob/sayzon-f35-v7-exp.1/Multiprotocol/SGF22_nrf24l01.ino#L113-L114) are **fixed**. The SGF22 row-index calculation appears consistent with `08`/`92` selecting row 4, but **one original stock TX is insufficient** to prove derivation or packet field semantics. Would Pascal prefer (i) an expressly fixed-ID engineering subtype while learning the format, (ii) more ID-capture work before any upstream PR, or (iii) a different protocol structure? I can run focused tests and revise the implementation in whichever direction is appropriate.

### D. Evidence missing for a broader release

I can capture and provide additional experiments if directed. My current limits are **not knowing enough to trust my XN297 ACK/CRC interpretation**, lack of an independently captured second stock transmitter identity, **no complete, time-correlated multi-channel acquisition trace or independently captured second stock transmitter**, and no systematic motor-safe loss-of-link or telemetry measurements. Rather than guessing at these details, I would appreciate guidance on **which single experiment would be decisive first**.

## 6. Safety and scope

**Experimental pre-release only.** The binary targets the **external STM32 IRX4 Plus MultiModule**, *not* the Q X7 radio firmware or receiver. Never infer a broadly safe/flyable link from normal surface-control response. Disconnect the EDF/motor during initial capture, cancel, and recovery tests. Keep this fork PR draft until basic framework, failsafe, and ID behavior have been reviewed.

## 7. Minimal potential reviewer response

The most useful first response is whether **(A)** the empty enhanced ACK / direct TX→RX approach is architecturally correct and **(B)** PR #4's explicit cancellation state can be integrated cleanly without the remaining A1 counter=2 ambiguity. A recommendation for one decisive next bench test is much more helpful than a general request for “more logs”; I'd be glad to generate precisely the required captures, including µs timing, from the actual aircraft.

**Supporting context:** [full reverse-engineering notes](SAYZON_F35_REVERSE_ENGINEERING.md) · [release notes, hash and hardware requirements](SAYZON_F35_V7_RELEASE.md) · [v7 exact source tag](https://github.com/avgbob/DIY-Multiprotocol-TX-Module/tree/sayzon-f35-v7-exp.1)
