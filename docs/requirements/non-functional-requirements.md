<!-- CONTRACT: quality attributes the system must meet, with a measurable target where
     possible. "Fast" and "user-friendly" are not requirements — a number or a
     testable condition is. Delete the _EXAMPLE_ row once you've seen the shape. -->

# Non-Functional Requirements

Convention: `NFR-###`. Category is one of: Performance, Security, Reliability,
Availability, Scalability, Usability, Maintainability, Compliance, Observability
(add categories as needed — keep the list here in sync). Priority: `Must` / `Should`
/ `Could`. Status as in functional-requirements.md.

| ID | Category | Requirement | Measurable Target | Priority | Status |
|---|---|---|---|---|---|
| NFR-001 | Reliability | Main node retries a reliable LoRa send before giving up | 2 retries (reduced from RadioHead's library default of 3) | Should | Implemented |
| NFR-002 | Performance | Main node waits for an ACK on a reliable LoRa send before treating it as failed | 400 ms timeout (measured on-air time at SF10/CR 4-5/BW 125 kHz is ~207-327 ms) | Should | Implemented |
| NFR-003 | Reliability | Main node retries SD-card initialization at boot before declaring the card failed | Up to 3 attempts, 500 ms apart | Should | Implemented |
| NFR-004 | Reliability | Main node recovers from a failed SD write by re-initializing the card | One re-init-and-retry cycle before giving up on that write | Should | Implemented |
| NFR-005 | Availability | Main node waits for a USB host at boot before proceeding without one | <= 10 s | Could | Implemented |
| NFR-006 | Observability | Main node's TFT keeps a scrolling log of recently received messages | 11 lines, up to 161 characters each | Could | Implemented |
| NFR-007 | Reliability | Once battery-powered (FR-016), main node operates on its battery, unserviced, in the field | TBD | Must | Proposed |

<!-- If you genuinely can't state a measurable target yet, write TBD in that column
     rather than a vague phrase — TBD is honest and searchable; "reasonably fast"
     just looks finished. -->
