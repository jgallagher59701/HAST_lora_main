<!-- CONTRACT: actor-driven flows through the system. Each use case is what ties
     functional requirements together into something a real user actually does.
     Delete the _EXAMPLE_ use case once you've seen the shape. -->

# Use Cases

Convention: `UC-###`. Each use case follows the template below. Link related `FR-###`
/ `NFR-###` IDs at the bottom so `/trace` can find them in both directions.

---

## UC-001 — Main node receives and logs a leaf node's reading

**Actor(s):** Leaf node (sender, autonomous); Main node (receiver, continuously
listening)

**Trigger:** A leaf node's reliable LoRa datagram carrying a `data_message` or `text`
message arrives while the main node's radio is listening

**Preconditions:** Main node is powered and initialized (radio and RTC up); SD card
may or may not be available — its absence is tolerated, not fatal

**Main flow:**
1. Main loop detects an available datagram (FR-001)
2. Current RTC time is read/printed
3. Message is decoded (sender address, recipient address, message ID, header)
4. Message type is dispatched to `data_message` or `text`
5. Message is pretty-printed to USB serial, along with LoRa link-quality metrics
   (RSSI, SNR, packet counts) (FR-012)
6. Message is logged to the main node's own SD card (FR-002, FR-003)
7. For a `data_message`, a line is rendered on the TFT scrolling display (FR-009)
8. The underlying reliable-datagram layer sends the link-level acknowledgment as
   part of receiving the message

**Alternate / exception flows:**
- SD card not initialized, or a write fails → main node re-initializes the card once
  and retries; if that also fails, the write is skipped without halting the loop
  (FR-011); a persistent fault is reflected via a latched status LED (FR-010)
- Unrecognized message type → raw bytes are dumped to USB serial only; not logged to
  SD or shown on the TFT

**Postconditions:** Reading appears in the main node's own CSV log and serial stream;
TFT reflects the latest reading (for data messages); the leaf node's send has been
acknowledged at the radio layer

**Related requirements:** FR-001, FR-002, FR-003, FR-008, FR-009, FR-010, FR-011,
FR-012, IC-003, IC-004, IC-006, IC-007, IC-008

---

## UC-002 — Main node answers a leaf node's time-sync request

**Actor(s):** Leaf node (requester); Main node (network time authority)

**Trigger:** A leaf node sends a `time_request` reliable datagram

**Preconditions:** Main node's RTC is running and was previously set (via the
compile-time `ADJUST_TIME` build flag, or the companion `DS3231-set-time` utility)

**Main flow:**
1. Main loop detects and decodes the incoming datagram (as in UC-001, steps 1-3)
2. Message type is dispatched to `time_request`
3. Request is pretty-printed, shown on the TFT, and logged to the SD card regardless
   of how the reply turns out (FR-005)
4. A `time_response` is built from the main node's address and its RTC's current
   time (FR-004, FR-006)
5. The response is sent back as a reliable (acknowledged) datagram; the ack/no-ack
   outcome is logged to the TFT and USB serial

**Alternate / exception flows:**
- Reply is not acknowledged after the reliable-datagram layer's built-in retries →
  outcome is logged as "no ack"; the loop continues without further retry at the
  application level

**Postconditions:** The requesting leaf node may now resynchronize its clock
(leaf node's own FR-005); the main node's log shows the request regardless of
whether the reply was acknowledged

**Related requirements:** FR-004, FR-005, FR-006, FR-007, FR-008, NFR-001, NFR-002,
IC-002, IC-003, IC-004

---

<!-- Copy the template above for each new use case. Sequential UC-### heading,
     same subsection order, so /trace can parse it. -->
