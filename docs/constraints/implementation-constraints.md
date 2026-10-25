<!-- CONTRACT: things that constrain HOW the system can be built, independent of
     what it must do. These are boundaries, not preferences — if a plan would cross
     one, that's a flag, not a quiet redesign. Delete the _EXAMPLE_ row once you've
     seen the shape. -->

# Implementation Constraints

Convention: `IC-###`. Category is one of: Technology/Platform, Infrastructure/
Deployment, Regulatory/Compliance, Organizational/Team, Third-Party/Integration,
Budget/Timeline (add categories as needed).

| ID | Category | Constraint | Rationale | Impact on design |
|---|---|---|---|---|
| IC-001 | Technology/Platform | Firmware's active, supported build target is the Adafruit Feather M0 (SAMD21) | Current default PlatformIO environment and board-select branch in `src/main-node.cc` | RocketScream Mini Ultra Pro and ESP8266 NodeMCU board-select branches exist in source but are not currently working/maintained targets; see `NFR-008`/`NFR-009` for the (low-priority) plan to restore portability to them — don't remove that code as dead, but don't treat it as a live target either |
| IC-002 | Technology/Platform | Main node's RTC hardware is a DS3231 (battery-backed) on the active build, with DS1307 as a compile-time fallback | Determines how network time is kept and distributed (FR-006, FR-007) | Any future board port must provide equivalent battery-backed RTC behavior |
| IC-003 | Regulatory/Compliance | LoRa radio must operate within the US ISM band (902-928 MHz); default channel is 902.3 MHz | FCC Part 15 unlicensed-operation requirement — same constraint as the leaf node's `IC-002` | Frequency/channel plan is constrained to this band; deploying outside the US would need re-certification |
| IC-004 | Infrastructure/Deployment | Main node's LoRa radio parameters (frequency, spreading factor, bandwidth, coding rate) must match the leaf nodes' for the link to work at all | Point-to-point radio link; today each side hard-codes its own copy of these values independently | Any change to these parameters must be coordinated with the `HAST_leaf_node` repo, not made unilaterally in this one |
| IC-005 | Infrastructure/Deployment | Main node supports at most 15 addressed leaf nodes (addresses 1-15); address 0 is reserved for the main node | Mirrors the leaf node's `IC-005` awake-time-budget math | Bounds the node-addressing scheme on the main-node side as well |
| IC-006 | Third-Party/Integration | Core hardware access is through fixed library choices: RadioHead (`RHReliableDatagram`/`RH_RF95`) for the LoRa reliable-datagram transport, SdFat for the SD card, RTClib for the RTC, Adafruit ST7735/ST7789 + GFX/BusIO for the TFT | Pinned dependencies in `platformio.ini` | Protocol/timing behavior (acks, retries, timeouts) is bounded by what RadioHead exposes; swapping any of these libraries is a significant change, not a drop-in |
| IC-007 | Third-Party/Integration | The leaf-node/main-node wire message protocol is defined in a shared library, `soil_sensor_common`, vendored into both repos as a pinned git submodule | Keeps both sides of the protocol byte-compatible | A protocol change must update the submodule and be re-pinned in both `HAST_leaf_node` and `HAST_lora_main` together — never edited independently in one repo |
| IC-008 | Third-Party/Integration | Maximum single LoRa message size is bounded by RadioHead's `RH_RF95_MAX_MESSAGE_LEN` (251 bytes) | Hard limit imposed by the radio driver | All message struct definitions in `soil_sensor_common` must fit within this bound |
| IC-009 | Infrastructure/Deployment | Main node is currently mains-powered; no low-power standby cycle exists — the radio listens continuously | Present deployment assumption; the firmware has no sleep/standby code path today | Will be superseded once `FR-016` (battery-powered, duty-cycled main node) is implemented — until then, any plan assuming a main-node awake-time budget (see the leaf node's `IC-005`) is aspirational, not yet true of the shipped firmware |

<!-- A constraint is not the same as a non-functional requirement: an NFR describes
     a quality the system should have (and can trade off); a constraint is a boundary
     condition that plans must work within, full stop. If it's negotiable, it's
     probably an NFR with a priority, not a constraint. -->
