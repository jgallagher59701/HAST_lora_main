# HAST_lora_main

Code for the HAST main node — the hub that receives LoRa data from one or more
leaf nodes, shows a live summary on a small TFT, and keeps the full detail in
a log on its SD card and over USB serial.

## What it does

The main node runs on an Adafruit Feather M0 (LoRa) — or a RocketScream Mini
Ultra Pro V3 / ESP8266 NodeMCU, which the code also supports via compile-time
board flags. Unlike a leaf node, it doesn't sleep or run on a schedule: once
booted, it sits in `loop()` listening for incoming LoRa messages and reacts to
whatever arrives.

For every message received from a leaf node, it:

1. Reads the current time from its own RTC (DS3231/DS1307) to timestamp the
   event locally.
2. Decodes the message by type (`data message`, `time request`, `text`, or
   `time response`/`join` types not yet implemented here — see Message types
   below) and prints a decoded, human-readable version to the USB serial port.
3. Appends a line to a CSV log file (`Sensor_data.csv`) on its SD card —
   `data message` and `time request` rows are the raw (undecoded-to-physical-
   units) field values; see **What's in the log** below.
4. Updates the TFT display with a one-line summary — but only for `data
   message` and `time request` types; `text` messages and unrecognized
   messages never appear on screen (see **On the display vs. in the log**).
5. For a `time request`, replies to the requesting leaf node with the main
   node's current RTC time, and reports on serial/TFT whether that reply was
   acknowledged.

The onboard LED lights while a message is being handled, and latches solid on
any SD card fault as a simple persistent "something's wrong" indicator — there
is no blink-code scheme here the way there is on the leaf node.

### Message types

| Type | Handled? | Logged to SD | Shown on TFT |
|---|---|---|---|
| `data_message` | Yes | Yes | Yes |
| `time_request` | Yes (and replies) | Yes | Yes |
| `text` | Yes | Yes | No |
| `join_request`/`join_response` | Only if built with `SUPPORT_JOIN` | — | — |
| Unrecognized type | Dumped as raw hex to serial only | No | No |

## Loading the code

This is a PlatformIO project. The default environment is
`adafruit_feather_m0`; `zeroUSB` / `debugZeroUSB` target a RocketScream board,
and `esp12e` targets the original ESP8266 NodeMCU prototype. Which board
pinout the firmware actually uses is selected by the `FEATHER_M0` /
`ROCKET_SCREAM` / `BUILD_ESP8266_NODEMCU` build flags in `platformio.ini`
(`[common_env_data]`), not by the PlatformIO environment name alone — check
that flag matches the environment you're building before flashing hardware
other than the Feather M0.

Other build-time settings worth knowing about in `platformio.ini`:

| Flag | Purpose |
|---|---|
| `VERSION` | Firmware version string printed to serial at boot |
| `FREQUENCY` | LoRa center frequency in MHz (must match the leaf nodes') |
| `ADJUST_TIME` | 1 = set the RTC to the sketch's compile time at boot (see the comment above it in `main-node.cc` for the two-flash procedure); 0 = normal boot, trust the RTC's battery-backed time |
| `SUPPORT_JOIN` | Enables the (currently unused) join-request/response message types |

## What's in the log

The SD card log is a single, append-only file, `Sensor_data.csv` — unlike the
leaf node, the main node does not roll to a new numbered file per boot, so a
device that has rebooted several times will have several `# Start Log` /
header blocks in the same file. The header line is:

```
Node, Message, Time, Battery V, Last TX Dur ms, Temp C, Hum %, Status
```

That header describes the columns of a `data message` row, which is what most
rows are. Its fields come straight from the leaf node's `data_message_t`
packet, **unscaled**:

| Column | Raw units |
|---|---|
| Node | Leaf node's one-byte address |
| Message | Leaf node's message sequence number |
| Time | Unix epoch seconds, from the *leaf* node's RTC at the moment it sampled |
| Battery V | Battery voltage × 100 (divide by 100 for volts) |
| Last TX Dur ms | Duration of the leaf node's previous LoRa transmit, in ms |
| Temp C | Temperature × 100 (divide by 100 for °C) |
| Hum % | Relative humidity × 100 (divide by 100 for %) |
| Status | The leaf node's status bitmask, hex — see **Status byte**, below |

`time_request` and `text` rows are also appended to this same file, but their
field layout doesn't match the header above — a `time_request` row is
`time request, <node>`, and a `text` row is `<node>, <message text>`. A tool
reading this CSV needs to branch on column count (or re-derive the message
type) rather than assuming every row is a `data message`.

### Status byte

The status byte is set by the *leaf* node and simply passed through here. See
`HAST_leaf_node`'s README for the authoritative bit table; the short version:
the low nibble reports per-cycle faults (LoRa send/ack, SD write/wakeup) and
is cleared every leaf-node cycle, while the high nibble reports boot-time
faults (SD/LoRa/sensor init, SD filename exhaustion) and persists for the
life of that leaf node's boot. `0x00` means no errors.

## On the display vs. in the log

The TFT is a quick-glance summary, not a record — a lot of what's captured on
SD and serial never makes it to the screen:

- **Only minutes:seconds, not a date or hour.** The TFT line shows `mm:ss` —
  the minute and second the main node *received* the message, with no hour
  and no date. The log's `Time` column is a full Unix timestamp taken from
  the *leaf* node's clock, which is what you'd actually use to correlate
  readings across a longer run.
- **Rounded/truncated values, not raw ones.** The TFT prints temperature to
  one decimal and battery to two, but humidity is integer percent — truncated
  by integer division, not rounded. The log keeps the full raw ×100 integer
  for all three, so finer precision only exists in the log.
- **No message sequence number or TX duration on screen.** `Message` (the
  leaf node's send counter, useful for spotting dropped messages) and `Last
  TX Dur ms` are logged but never displayed.
- **No radio link quality on screen or in the log.** After every received
  message the firmware prints RSSI, SNR, and cumulative good/bad packet
  counts (`print_rfm95_info()`) — but only to USB serial. This is not written
  to the SD card log and not shown on the TFT, so it's only visible with a
  serial monitor attached live.
- **`text` messages don't appear on the TFT at all.** They're decoded to
  serial and appended to the SD log, but there is no TFT update for this
  message type — only `data_message` and `time_request` get a screen line.
- **Raw packet framing is serial-only.** The `len/from/to/id/header` values
  RadioHead reports for every received datagram, and the raw hex dump for any
  unrecognized message type, are printed to serial only — never logged or
  shown on the TFT.
- **Boot-time diagnostics are serial-only.** Firmware version, SD card
  init/retry status, LoRa init/frequency, and RTC startup are all printed to
  serial during `setup()`; none of it is written to the SD log (the log file
  only gets a `# Start Log` + header line) or shown on the TFT, which just
  redraws its static column header.
- **A latched SD-card fault shows only as a solid LED.** There's no on-screen
  or logged message for it beyond whatever was last printed to serial when
  the fault occurred — the LED is the only persistent indicator.

## Capturing the serial stream

`PyNodeLog.py` reads the main node's USB serial port and appends each line,
timestamped with the host's wall-clock time, to `test_data.csv`. It's a
convenience for capturing everything that scrolls past on serial — including
the RSSI/SNR and boot diagnostics that never reach the SD card — during a
bench session; edit `usb_port` in the script for your OS/port before running
it.
