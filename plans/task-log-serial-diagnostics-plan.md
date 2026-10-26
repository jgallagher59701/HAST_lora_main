# Task Plan: Log the serial-only diagnostic fields to the SD card

**Task:** TASK-007
**Status:** Draft
**Created:** 2026-10-25

## Summary

Persist three groups of diagnostic information that `src/main-node.cc`
currently only sends to USB serial — LoRa link-quality metrics, raw
RadioHead framing fields, and boot-time diagnostics — to the SD card log, so
they survive a bench session where nobody was watching the serial monitor
live.

## Motivation

Found while writing the `HAST_lora_main` README: RSSI/SNR/packet counts
(`print_rfm95_info()`), the raw `len/from/to/id/header` values RadioHead
reports per datagram, and the `setup()`-time boot diagnostics (firmware
version, SD/LoRa/RTC init results) are all printed via `print()`/`Serial`
only. None of it reaches `log_data()`/the SD card, so once a session scrolls
past on serial without being captured, it's gone for good.

## Related requirements / constraints

- **FR-012** — "Main node reports LoRa link-quality metrics (RSSI, SNR,
  good/bad packet counts) over USB serial for each received application
  message," `Status: Implemented`. This currently documents link-quality
  reporting as serial-only, by design. This task extends that beyond what
  FR-012 currently states. Recommend running `/new-requirement` to broaden
  FR-012 (or add a new FR) once this is implemented, so the requirement docs
  stay in sync.
- **UC-001**, step 5, says link-quality metrics are reported "to USB serial" —
  same scoping note applies here.
- No requirement currently covers the raw framing fields or boot diagnostics
  at all (serial or otherwise) — adding SD logging for those isn't in tension
  with anything written down, just new ground.
- **TASK-005** (give each message type a self-consistent CSV log row shape) —
  this task adds *new* row shapes to the same log file, so it's worth settling
  TASK-005's discriminator/column-shape decision first, or at least choosing a
  shape here that TASK-005 can absorb rather than conflict with. See **Open
  questions**.

## Scope

**In scope:**
- Logging LoRa link-quality metrics (RSSI, SNR, `rxGood`/`rxBad`) to the SD
  card alongside the existing serial print, for every received application
  message (`data_message`, `text`, `time_request`).
- Logging the raw RadioHead framing fields (`len`, `from`, `to`, `id`,
  `header`, decoded message type) to the SD card alongside the existing
  serial print, once per received datagram.
- Logging a one-time boot-diagnostics summary (firmware version, SD card
  init/retry outcome, LoRa radio init/frequency-set outcome, RTC startup
  status) to the SD card at the end of `setup()`.

**Out of scope:**
- Anything shown on the TFT — unaffected by this task.
- The unrecognized-message hex dump, which `docs/requirements/use-cases.md`
  already documents as serial-only by design — not touched here.
- Resolving TASK-005 itself — this task assumes *a* row-shape decision gets
  made (here or there) and follows it; it doesn't redesign the existing
  `data_message` row.
- Changing `print_rfm95_info()`'s or `setup()`'s existing serial output —
  this task adds SD logging alongside it, not instead of it.

## Approach

1. **Link-quality metrics:** add a sibling to `print_rfm95_info()` — e.g.
   `rfm95_info_to_string()` returning a formatted string the way
   `data_message_to_string()` does — and call `log_data(FILE_NAME, ...)` with
   it right after each of the three existing `print_rfm95_info()` call sites
   in `loop()` (`data_message`, `text`, `time_request` cases).
2. **Raw framing fields:** format the `len/from/to/id/header/type` values
   already captured right after `recvfromAck()` succeeds (where they're
   currently passed straight to `print()`) into a second string, and log it
   once per received datagram, before the `switch` on message type.
3. **Boot diagnostics:** accumulate the pass/fail outcomes already computed in
   `setup()` (`sd_card_status`, the `rf95_manager.init()` result, the RTC
   `real_time_clock.begin()` result) into one line, and call `log_data()` with
   it at the end of `setup()`, after `write_header()` has run so it lands
   after the file's `# Start Log` header.
4. Use a simple leading discriminator column on every new row (e.g. `LINK,
   ...`, `FRAME, ...`, `BOOT, ...`) so these rows are trivially distinguishable
   from `data_message`/`time_request`/`text` rows by any reader, independent
   of how TASK-005 resolves column-count ambiguity for the existing three
   types. If TASK-005 lands first with a different discriminator convention,
   reconcile this task's new row prefixes with it rather than inventing a
   second convention.
5. Add native unit tests for any pure string-formatting helper extracted in
   steps 1–2 (e.g. `rfm95_info_to_string()`), under `test/native/`, following
   the existing `messages.cc`-style `_to_string()` test pattern. The boot
   summary in step 3 is harder to unit-test in isolation since it reads
   hardware init results directly in `setup()`; cover it via the manual bench
   check in **Verification** instead.

Three call sites change for link-quality metrics (one per message type) plus
one for framing fields plus one for the boot summary — five total. Completeness
check: `grep -n "print_rfm95_info()" src/main-node.cc` should show the same
three call sites each now paired with a `log_data()` call directly below it;
`grep -n "Received length:" src/main-node.cc` should show one site similarly
paired.

## Verification

- `pio run -e adafruit_feather_m0` builds clean.
- `pio test -e native` passes, including any new formatter tests.
- `pio check` reports no new warnings.
- Bench test with a leaf node: confirm `Sensor_data.csv` gains `LINK`/`FRAME`
  rows for each received message and a `BOOT` row at startup, while the
  existing serial output and `data_message`/`time_request`/`text` rows are
  unchanged.

## Risks

- `log_data()` opens and closes the SD file with interrupts disabled
  (`noInterrupts()`/`interrupts()`) for each call. This task roughly doubles
  or triples the number of `log_data()` calls per received message (one
  existing data/text/time-request row, plus a new `LINK` row, plus
  occasionally a `FRAME` row) — worth measuring the added per-message latency
  on actual hardware, since the reliable-datagram layer's ack timing
  (`rf95_manager.setTimeout(400)`) is latency-sensitive.
- Faster SD log growth: a `data_message` arriving every reporting interval
  now produces 2–3 log lines instead of 1, roughly doubling or tripling the
  file's growth rate over a long deployment.
- New row shapes land in the same file TASK-005 is meant to clean up — if
  both tasks are implemented out of order or by different conventions, the
  log could end up with *more* inconsistent row shapes rather than fewer.

## Open questions

- Should TASK-005 be implemented first (or its row-shape convention decided)
  so this task's `LINK`/`FRAME`/`BOOT` rows use the same discriminator
  convention from the start, rather than needing to be revisited later?
  Recommend the user decide the ordering before implementation begins.
- Should FR-012 be reworded to include SD logging, or tracked as a new FR?
  Recommend the user decide via `/new-requirement` before/alongside
  implementing this task.
- Is per-message `LINK`/`FRAME` logging actually wanted at the volume it
  implies (every received datagram, not just data readings), or should it be
  sampled/throttled? Not resolved here — flagging since it drives the SD
  growth-rate risk above.
