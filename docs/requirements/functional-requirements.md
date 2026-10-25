<!-- CONTRACT: what the system must DO. One row per requirement. IDs are permanent —
     never renumbered or reused. Dropped requirements get Status: Deprecated /
     Superseded, not deleted. Delete the _EXAMPLE_ row once you've seen the shape. -->

# Functional Requirements

Convention: `FR-###`, zero-padded, assigned sequentially by next-highest-number.
Priority: `Must` / `Should` / `Could` (MoSCoW). Status: `Proposed` / `Approved` /
`Implemented` / `Deprecated` / `Superseded by FR-0XX`.

| ID | Requirement | Priority | Related Use Cases | Status |
|---|---|---|---|---|
| FR-001 | Main node listens continuously on LoRa for reliable datagrams from leaf nodes | Must | UC-001, UC-002 | Implemented |
| FR-002 | Main node receives a data reading (`data_message`) from a leaf node, logs it to its own SD card, prints it to USB serial, and displays it on its TFT | Must | UC-001 | Implemented |
| FR-003 | Main node receives a free-text (`text`) message from a leaf node and logs it to the SD card and USB serial | Should | UC-001 | Implemented |
| FR-004 | Main node receives a `time_request` from a leaf node and replies with a `time_response` carrying its RTC's current time, as a reliable (acknowledged) datagram | Must | UC-002 | Implemented |
| FR-005 | Main node logs each `time_request` it receives to the SD card and TFT regardless of whether its reply is acknowledged | Should | UC-002 | Implemented |
| FR-006 | Main node maintains the network's authoritative time using a battery-backed hardware RTC (DS3231, with DS1307 as a compile-time fallback) | Must | UC-002 | Implemented |
| FR-007 | Main node's RTC can be set to the firmware's build time via a compile-time `ADJUST_TIME` flag, for initial/field clock setup | Should | UC-002 | Implemented |
| FR-008 | Each main node has a unique, fixed node address (0) distinguishing it from all leaf nodes | Must | UC-001, UC-002 | Implemented |
| FR-009 | Main node displays received leaf-node readings and time requests as a scrolling log on an attached TFT display | Should | UC-001, UC-002 | Implemented |
| FR-010 | Main node reports a persistent hardware/SD-card fault via a latched status LED, distinguishable from normal receive-activity blinking | Should | | Implemented |
| FR-011 | Main node retries SD-card initialization at boot, and re-initializes/retries once on a failed write, before treating the card as unusable | Should | | Implemented |
| FR-012 | Main node reports LoRa link-quality metrics (RSSI, SNR, good/bad packet counts) over USB serial for each received application message | Could | UC-001, UC-002 | Implemented |
| FR-013 | Main node supports admitting a new leaf node via a join-request/join-response exchange that assigns it a node address | Could | | Proposed — message types are defined and unit-tested in the shared `soil_sensor_common` library, but the main-node handler (`src/main-node.cc`, `case join_request`) is an unimplemented stub, gated behind a `SUPPORT_JOIN` build flag that is off by default |
| FR-014 | Main node handles a leaf-node-reported hardware/SD-card error message | Should | | Proposed — no handler exists for the `error` message type (falls through to the generic unrecognized-message branch); mirrors the leaf node's own FR-010, which is also still Proposed |
| FR-015 | A host-side companion script (`PyNodeLog.py`) tails the main node's USB serial output and persists each line, timestamped, to a CSV file on a PC | Could | | Implemented (host-side tooling, not firmware) |
| FR-016 | Main node can operate on battery power (e.g. D-cell), duty-cycling between low-power standby and LoRa-listening in synchrony with the leaf nodes' transmit schedule, for field deployments without mains power | Must | | Proposed — no standby/duty-cycle code exists today; main node currently listens continuously (see `IC-009`) |

<!-- Add new rows via /new-requirement, or by hand — keep the table format. Each
     requirement should be a single testable statement, not a paragraph. If it needs
     a paragraph, it's probably a use case (docs/requirements/use-cases.md) with this
     as one of its acceptance criteria. -->
