# Recovered SAYZON F-35 / XN297Dump attachments — October 2–3, 2026

Source: the original uploaded files preserved in the ChatGPT user's file library (not generated packet examples). All files are preserved byte-for-byte under `original_uploads/`; see `SHA256SUMS.txt`.

Important caveats:
- Most captures are fixed-channel logs, especially C=24 and C=28; they do **not** independently prove all four hopping frequencies, even if previous work derives a hop set.
- A `RX: NNNus` value is the tool-reported interval; some large/reset values and malformed/incomplete records exist. A decoded record is not automatically proof of the transmitted packet's origin.
- A visible `P(0)=` next to A0/A1 is an observed *zero-length enhanced packet*, not independent proof of the native receiver ACK semantics or its causality.
- Do not conflate the single factory transmitter RF identity with two different physical aircraft being controlled by the same modified module.
- Only files classified `raw-RX-capture` contribute to the enhanced packet count. Counts are lexical summaries, **not** deduplicated or verified unique events; repeated/overlapping logs may exist.
- Some files are raw RF scanner configuration logs; two files are notes or embedded markdown rather than original plain serial RX logs.

> **Public sample captures:** Six original uploaded logs have been committed byte-for-byte below. The remaining 29 original attachments are preserved in the separate complete recovery ZIP provided to the experimenter for posting to upstream issue #1196.

**Direct links (unaltered original uploads):**

- [2026-10-02 03:01:08 — bind A0, zero-length packet and A1](SAYZON_XN297_20261002_030108_bind_ACK_and_A1.txt)
- [2026-10-02 03:49:30 — A0 to A1 transition](SAYZON_XN297_20261002_034930_A0_to_A1.txt)
- [2026-10-02 03:58:44 — A0-only capture](SAYZON_XN297_20261002_035844_A0_only.txt)
- [2026-10-02 02:52:14 — flight-channel traffic, C=24](SAYZON_XN297_20261002_025214_flight_CH24.txt)
- [2026-10-02 15:24:21 — E0/F0 one-byte enhanced response examples](SAYZON_XN297_20261002_152421_flight_E0_F0_probe.txt)
- [2026-10-03 14:15:18 — later control/trim sweep capture](SAYZON_XN297_20261003_141518_control_trim_sweep.txt)

## Recommended reviewer reading order

1. `Pasted text(20261002-030108).txt` — bind-channel capture with A0, three P(0) lines, A1, and some flight-address records (around lines 590–602).
2. `Pasted text(20261002-034930).txt` — clearer A0→A1 transition (around lines 750–762).
3. `Pasted text(20261002-035844).txt` — 1,009 enhanced A0 records; no A1 in this file. **Receiver power state unverified from file alone.**
4. `Pasted text(20261002-025214).txt` — raw flight packets at C=24, including a one-byte `F0` (around line 611).
5. `Pasted text(20261002-152421).txt` — flight records and 1-byte E0/F0 observations on C=24.
6. `Pasted text(20261003-141518).txt` — later stick/trim-related capture with variable payload values.

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

For maintainer review, use the selected captures above first; if a timing/ACK question requires broader context, consult the complete original archive here. The `P(1)=E0/F0` lines support one-byte response traffic but do **not** establish voltage semantics.