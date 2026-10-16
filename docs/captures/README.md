# Recovered SAYZON F-35 / XN297Dump attachments — October 2–3, 2026

Source: the original October 2–3 uploads from the experimenter's archived packet logs (not generated examples). The complete [35-file recovery ZIP attached to Pascal's issue #1196](https://github.com/user-attachments/files/33471553/SAYZON_F35_REDISCOVERED_XN297DUMP_LOGS_20261002-03.zip) preserves the original files byte-for-byte under `original_uploads/`; see its `SHA256SUMS.txt`. Six selected exact originals are also committed directly in this `docs/captures/` directory.

Important caveats:
- Most captures are fixed-channel logs, especially C=24 and C=28; they do **not** independently prove all four hopping frequencies, even if previous work derives a hop set.
- A `RX: NNNus` value is the tool-reported interval; some large/reset values and malformed/incomplete records exist. A decoded record is not automatically proof of the transmitted packet's origin.
- A visible `P(0)=` next to A0/A1 is an observed *zero-length enhanced packet*, not independent proof of the native receiver ACK semantics or its causality.
- Do not conflate the single factory transmitter RF identity with two different physical aircraft being controlled by the same modified module.
- Only files classified `raw-RX-capture` contribute to the enhanced packet count. Counts are lexical summaries, **not** deduplicated or verified unique events; repeated/overlapping logs may exist.
- Some files are raw RF scanner configuration logs; two files are notes or embedded markdown rather than original plain serial RX logs.

> **Evidence availability:** Six original uploaded logs have been committed byte-for-byte below. **All 35** original attachments (including the six) are already [attached to upstream issue #1196 as a checksum-indexed ZIP](https://github.com/user-attachments/files/33471553/SAYZON_F35_REDISCOVERED_XN297DUMP_LOGS_20261002-03.zip). No additional upload is needed to access them.

**Direct links (unaltered original uploads):**

- [2026-10-02 03:01:08 — bind A0, zero-length packet and A1](SAYZON_XN297_20261002_030108_bind_ACK_and_A1.txt)
- [2026-10-02 03:49:30 — A0 to A1 transition](SAYZON_XN297_20261002_034930_A0_to_A1.txt)
- [2026-10-02 03:58:44 — A0-only capture](SAYZON_XN297_20261002_035844_A0_only.txt)
- [2026-10-02 02:52:14 — flight-channel traffic, C=24](SAYZON_XN297_20261002_025214_flight_CH24.txt)
- [2026-10-02 15:24:21 — E0/F0 one-byte enhanced response examples](SAYZON_XN297_20261002_152421_flight_E0_F0_probe.txt)
- [2026-10-03 14:15:18 — **isolated X-PILOT three-position mode sweep** (not a trim-button mapping)](SAYZON_XN297_20261003_141518_control_trim_sweep.txt)

## Recommended reviewer reading order

1. `Pasted text(20261002-030108).txt` — bind-channel capture with A0, three P(0) lines, A1, and some flight-address records (around lines 590–602).
2. `Pasted text(20261002-034930).txt` — clearer A0→A1 transition (around lines 750–762).
3. `Pasted text(20261002-035844).txt` — 1,009 enhanced A0 records; no A1 in this file. **Receiver power state unverified from file alone.**
4. `Pasted text(20261002-025214).txt` — raw flight packets at C=24, including a one-byte `F0` (around line 611).
5. `Pasted text(20261002-152421).txt` — flight records and 1-byte E0/F0 observations on C=24.
6. `Pasted text(20261003-141518).txt` — **isolated X-PILOT mode-switch** transitions (byte 5: `18 → 10 → 14 → 18 → 14 → 10`); byte 6 stays `00`, trim tail stays `10 C2`. Do not use this recording to assign physical trim buttons.

## October 3 control and trim capture audit — October 15, 2026

**The four original logs below are distinct and must not be conflated.** The 14:15 file is in this GitHub folder; the other three are inside the [complete 35-file archive linked above](https://github.com/user-attachments/files/33471553/SAYZON_F35_REDISCOVERED_XN297DUMP_LOGS_20261002-03.zip). The user confirmed afterward that **X-PILOT was moved** during the first, mixed trim/button recording, so its byte-5 `10 ↔ 00` transitions must not be attributed to the "find aircraft" function.

| Original capture (PuTTY name) | Captured flight observations | Valid interpretation / limitation |
| --- | --- | --- |
| `Pasted text(20261003-135525).txt` — mixed trim/button sweep | 5,221 complete `P(9)` records; byte 6 remained `00`; trim tail examples `10 42 → 11 42`, `11 42 → 11 3E`, `F1 3D → F1 39`, `CF 35 → CE 35`; byte 5 moved `10 ↔ 00` | Bytes 7/8 do change with trim activity. **X-PILOT was also moved**, so byte 5's extra value does **not** identify "di-di"/find-aircraft. With EDF disconnected, an audible beep was not confirmed |
| `Pasted text(20261003-141227).txt` — additional trim sweep | 3,463 complete `P(9)` records; byte 7 `10 ↔ 0F`; byte 8 `42 ↔ 3E`; other trim changes also occur | Small reversible field changes exist, but physical direction-to-field mapping remains undetermined |
| `Pasted text(20261003-141518).txt` — **isolated X-PILOT** | 4,042 complete `P(9)` records (of which **4,041 are labeled `RX:`**, explaining the inventory count); byte 5 `18 → 10 → 14 → 18 → 14 → 10`; byte 6 `00`; byte 7/8 remain `10 C2` | **Mode evidence**, not evidence of trim or find-aircraft. The startup `X:` line contains one additional `P(9)` payload outside the `RX:` lexical count |
| `Pasted text(20261003-143145).txt` — additional control/trim sweep | 4,798 complete `P(9)` records; byte 5 `10`, byte 6 `00`, byte 7 `10`, byte 8 shows `C2 → 42 → BE → BA → B6 → B2 → B6 → BA → BE` | Byte 8 changes with control activity, including its high bit, but the bit `0x80` and packed field boundaries are not decoded |

**Packed trim:** Across the recordings, plausible byte-7 step sizes are `±1` and `±0x20`; plausible byte-8 step sizes are `±1` and `±4`, with an additional unexplained high-bit behavior. This is an **encoding hypothesis**, not a validated trim-axis map or an exact bitfield decoder. [Draft PR #5](https://github.com/avgbob/DIY-Multiprotocol-TX-Module/pull/5) tests one anonymous field at a time, gated, on top of the PR #4 refactor. It has **not** been flashed or bench-proven.

**Auxiliary flags:** A stricter archival scan of all 35 recovered files identified **100,352 complete nine-byte flight records across 30 files** (not independent unique transmissions): byte 6 was `00` in **89,772** and `20` in **10,580**. Neither `02` nor `04` was observed in those complete records. The archive contains fixed-channel and mixed-state sessions, so this does **not** eliminate a briefly pulsed command that was missed by the sniffing window. In particular, neither `02` nor `04` should be labeled a confirmed "find aircraft" or "stunt" command; `20` remains a candidate circling/loiter flag based on the stock button behavior and Chinese manual.

**Parsing caveat:** The inventory below records literal `RX ... Enhanced pid` lines, including other payload lengths and some unparseable records; the audit count above selects complete `P(9)` flight frames on the known flight address, and may include startup `X:` lines. These are different denominators, not inconsistent packet evidence. The full archive is necessary to independently reproduce the aggregate; the six inline examples are **not** sufficient for the 100,352 count. A standalone reproducible parsing script has **not** yet been checked into this fork.

## Inventory

| Original filename | Class | Bytes | Enhanced RX lines | A0 | A1 | P(0) | P(1)=E0/F0 |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: |
| `Pasted markdown(20261003-142209).md` | analysis or embedded notes | 301,614 | 0 | 0 | 0 | 0 | 0/0 |
| `Pasted text(20261002-023406).txt` | scan/config | 42,730 | 0 | 0 | 0 | 0 | 0/0 |
| `Pasted text(20261002-023650).txt` | analysis or embedded notes | 60,660 | 0 | 0 | 0 | 0 | 0/0 |
| `Pasted text(20261002-024638).txt` | raw-RX-capture | 87,854 | 1,010 | 0 | 0 | 0 | 0/0 |
| `Pasted text(20261002-025214).txt` | raw-RX-capture | 88,136 | 1,010 | 0 | 0 | 1 | 0/1 |
| `Pasted text(20261002-025351).txt` | raw-RX-capture | 89,128 | 1,041 | 0 | 0 | 32 | 0/0 |
| `Pasted text(20261002-025723).txt` | raw-RX-capture | 41,335 | 474 | 0 | 0 | 0 | 0/0 |
| `Pasted text(20261002-030108).txt` | raw-RX-capture | 84,535 | 950 | 594 | 4 | 3 | 0/0 |
| `Pasted text(20261002-031200).txt` | raw-RX-capture | 88,571 | 996 | 0 | 0 | 0 | 0/0 |
| `Pasted text(20261002-031635).txt` | raw-RX-capture | 548,311 | 6,134 | 0 | 0 | 0 | 0/0 |
| `Pasted text(20261002-031720).txt` | raw-RX-capture | 803,318 | 9,000 | 0 | 0 | 0 | 0/0 |
| `Pasted text(20261002-032019).txt` | raw-RX-capture | 330,080 | 3,721 | 0 | 0 | 0 | 0/0 |
| `Pasted text(20261002-032319).txt` | raw-RX-capture | 372,250 | 4,190 | 0 | 0 | 0 | 0/0 |
| `Pasted text(20261002-032529).txt` | raw-RX-capture | 179,593 | 2,005 | 0 | 0 | 0 | 0/0 |
| `Pasted text(20261002-032825).txt` | raw-RX-capture | 444,920 | 4,997 | 0 | 0 | 8 | 0/0 |
| `Pasted text(20261002-032949).txt` | raw-RX-capture | 232,161 | 2,596 | 0 | 0 | 2 | 0/0 |
| `Pasted text(20261002-033809).txt` | raw-RX-capture | 89,125 | 914 | 0 | 0 | 0 | 0/0 |
| `Pasted text(20261002-034930).txt` | raw-RX-capture | 84,493 | 968 | 756 | 12 | 0 | 0/0 |
| `Pasted text(20261002-035508).txt` | raw-RX-capture | 163,152 | 1,900 | 1,229 | 11 | 0 | 0/0 |
| `Pasted text(20261002-035844).txt` | raw-RX-capture | 84,946 | 1,009 | 1,009 | 0 | 0 | 0/0 |
| `Pasted text(20261002-040544).txt` | raw-RX-capture | 86,603 | 995 | 330 | 13 | 0 | 0/0 |
| `Pasted text(20261002-040813).txt` | raw-RX-capture | 757,549 | 8,862 | 6,247 | 24 | 0 | 0/0 |
| `Pasted text(20261002-144955).txt` | raw-RX-capture | 257,228 | 2,960 | 0 | 0 | 15 | 0/13 |
| `Pasted text(20261002-145444).txt` | raw-RX-capture | 292,078 | 3,351 | 0 | 0 | 4 | 0/2 |
| `Pasted text(20261002-150133).txt` | raw-RX-capture | 330,721 | 3,798 | 0 | 0 | 1 | 0/1 |
| `Pasted text(20261002-150643).txt` | raw-RX-capture | 465,614 | 5,347 | 0 | 0 | 0 | 0/2 |
| `Pasted text(20261002-151336).txt` | raw-RX-capture | 475,692 | 5,461 | 0 | 0 | 0 | 0/0 |
| `Pasted text(20261002-152421).txt` | raw-RX-capture | 754,201 | 8,676 | 0 | 0 | 29 | 32/1 |
| `Pasted text(20261002-153710).txt` | raw-RX-capture | 617,833 | 7,094 | 0 | 0 | 0 | 1/0 |
| `Pasted text(20261002-154150).txt` | raw-RX-capture | 329,656 | 3,784 | 0 | 0 | 0 | 0/0 |
| `Pasted text(20261003-135525).txt` | raw-RX-capture | 456,587 | 5,221 | 0 | 0 | 0 | 0/0 |
| `Pasted text(20261003-140813).txt` | scan/config | 20,797 | 0 | 0 | 0 | 0 | 0/0 |
| `Pasted text(20261003-141227).txt` | raw-RX-capture | 301,585 | 3,462 | 0 | 0 | 0 | 0/0 |
| `Pasted text(20261003-141518).txt` | raw-RX-capture | 351,906 | 4,041 | 0 | 0 | 0 | 0/0 |
| `Pasted text(20261003-143145).txt` | raw-RX-capture | 417,925 | 4,797 | 0 | 0 | 0 | 0/0 |

## Next step

For maintainer review, use the six selected captures above first; the [attached complete archive](https://github.com/user-attachments/files/33471553/SAYZON_F35_REDISCOVERED_XN297DUMP_LOGS_20261002-03.zip) provides the additional original control sweeps and timing context. The `P(1)=E0/F0` lines support one-byte response traffic but do **not** establish voltage semantics. Consult the [current upstream review packet](../SAYZON_F35_UPSTREAM_REVIEW_PACKET.md) for PR #4 hardware status and remaining ACK/cancellation questions.