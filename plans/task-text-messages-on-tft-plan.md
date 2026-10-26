# Task Plan: Display `text` messages on the TFT

**Task:** TASK-006
**Status:** Draft
**Created:** 2026-10-25

## Summary

Add a TFT line for received `text` messages, the one message type `loop()`
currently decodes and logs but never shows on screen.

## Motivation

Found while writing the `HAST_lora_main` README: `data_message` and
`time_request` both get a `tft_display_data()` line, but the `case text:`
branch in `src/main-node.cc` only calls `print()` (serial) and `log_data()`
(SD) — a `text` message is otherwise invisible unless someone is watching the
serial monitor live or later reads the SD log.

## Related requirements / constraints

- **FR-009** — "Main node displays received leaf-node readings and time
  requests as a scrolling log on an attached TFT display." This currently
  documents the TFT as covering `data_message` and `time_request` only; it
  does not mention `text`. This task extends behavior beyond what FR-009
  currently states. Recommend running `/new-requirement` to either broaden
  FR-009's wording or add a new FR once this is implemented, so the
  requirement docs stay in sync with what the firmware actually does.
- **FR-003** — "Main node receives a free-text (`text`) message from a leaf
  node and logs it to the SD card and USB serial." Unaffected: this task adds
  a TFT line alongside the existing SD/serial handling, it doesn't change it.
- **NFR-006** — TFT keeps a scrolling log, 11 lines of up to 161
  (`DATA_LINE_CHARS`) characters each. The new text line must fit this same
  budget.

## Scope

**In scope:**
- A new formatting function, e.g. `tft_text_message(const text_t *data,
  unsigned int min, unsigned int sec, char text[DATA_LINE_CHARS])`, declared
  in `include/TFTDisplay.h` and defined in `src/TFTDisplay.cc`, following the
  existing pattern of `tft_get_data_line()` / `tft_time_request()`.
- Wiring that function into the `case text:` branch of `loop()` in
  `src/main-node.cc`, calling `tft_display_data()` with its output — the same
  pattern already used for `data_message` and `time_request`.
- A native unit test for the new formatter under
  `test/native/test_tft/TFTDisplay_test.cc`, following the existing
  `test_tft_get_data_line()` pattern.

**Out of scope:**
- Any change to what's written to the SD log or serial for `text` messages —
  those are unaffected.
- Redesigning the TFT's scrolling-line mechanics (`MAX_LINE`, 10px line
  height, header, scrolling behavior in `tft_display_data()`) — the new line
  is just another call into the existing mechanism.
- Wrapping or paginating long text messages — `text_t.buf` can hold far more
  than fits on one un-wrapped TFT line (`tft.setTextWrap(false)`); this task
  truncates via `snprintf`, it does not add wrapping.
- The existing lack of a guaranteed null terminator on `text_t.buf` (also
  present in `text_message_to_string()` already) — not introduced or fixed by
  this task, just inherited.
- Updating FR-009's wording — left as a recommendation above, not auto-applied
  here, since requirement wording changes are a separate `/new-requirement`
  decision for the user to confirm.

## Approach

1. Add the declaration to `include/TFTDisplay.h`, next to `tft_time_request()`:
   ```cpp
   void tft_text_message(const text_t *data, unsigned int min, unsigned int sec, char text[DATA_LINE_CHARS]);
   ```
2. Implement it in `src/TFTDisplay.cc`, mirroring `tft_time_request()`:
   - Call `parse_text_message()` to get `node`, `length`, and the message
     buffer.
   - Format something like `"Text %u at %02u:%02u: %s"` (node, min, sec,
     message) via `snprintf()` into `text[DATA_LINE_CHARS]`. `snprintf`
     truncates safely if the message is longer than the buffer.
3. In `src/main-node.cc`'s `case text:` branch, after the existing
   `print()`/`log_data()` calls, add:
   ```cpp
   char text[DATA_LINE_CHARS];
   tft_text_message((text_t *)rf95_buf, real_time_clock.now().minute(), real_time_clock.now().second(), text);
   tft_display_data(text);
   ```
   matching the structure already used in `case time_request:`.
4. Add `test_tft_text_message()` to `TFTDisplay_test.cc`, built with
   `build_text_message()` test data, asserting the formatted string is
   non-null and contains the expected node number — matching the existing
   (intentionally light) assertion style of `test_tft_get_data_line()`.
5. Report the new FR-009 scope question to the user rather than silently
   editing `functional-requirements.md`.

Only one call site changes behavior (`case text:` in `loop()`), so
completeness here just means: that one branch now has a third call
(`tft_display_data()`) matching its two siblings, confirmed by inspection/diff
rather than a count.

## Verification

- `pio run -e adafruit_feather_m0` builds clean.
- `pio test -e native` passes, including the new `test_tft_text_message()`
  case.
- `pio check` reports no new warnings.
- Bench test: send a `text` message from a leaf node (or a short test
  harness) and confirm a line appears on the physical TFT, in addition to the
  existing serial/SD output, without disturbing the `data_message`/
  `time_request` lines already on screen.

## Risks

- `text_t.buf` is `TEXT_BUF_LEN` bytes (~245), far larger than a single
  un-wrapped TFT line can show — the visible portion will just be whatever
  fits before the screen edge, same truncation-by-display-width behavior the
  other TFT lines already have.
- If `text_t.buf` isn't null-terminated by the sender, `snprintf("%s", ...)`
  could read past the buffer the same way `text_message_to_string()` already
  risks today — this task doesn't add a new risk, but doesn't close the
  existing one either.

## Open questions

- Should FR-009 be reworded to include `text`, or should this be tracked as a
  new FR? Recommend the user decide via `/new-requirement` before/alongside
  implementing this task, so the requirement docs don't silently fall behind
  the firmware.
