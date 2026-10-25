<!-- This file is auto-loaded by Claude Code at the start of every session in this
     repo. It is a CONTRACT, not a log. If you're an editor here to update it, keep
     it short — it earns its place by being read every session, not by being complete. -->

# CLAUDE.md — `HAST_lora_main`

**What this project is:** This is the main node for the HAST soil sensor project

This repo plans work from written requirements, not from conversation memory or
assumption. Requirements, use cases, and constraints live in `docs/`. Plans that
Claude produces live in `plans/`. Every plan traces back to written IDs: a feature
plan cites FR/NFR/UC/IC IDs, a bugfix plan cites a `BUG-###`, a task plan cites a
`TASK-###` (bugfix and task plans may legitimately cite no FR/NFR/UC). If a *feature*
request has no requirement backing it, that's a signal to add one — not to infer one
silently.

## Read before planning anything

| File / folder | Holds |
|---|---|
| `docs/requirements/functional-requirements.md` | What the system must do — `FR-###` |
| `docs/requirements/non-functional-requirements.md` | Quality attributes & targets — `NFR-###` |
| `docs/requirements/use-cases.md` | Actor-driven flows — `UC-###` |
| `docs/constraints/implementation-constraints.md` | Hard boundaries on *how* — `IC-###` |
| `docs/decisions/DECISIONS.md` | Why past calls were made — `ADR-###` |
| `docs/bugs/BUG-LOG.md` | Deviations from intended behavior, report through root cause — `BUG-###` |
| `docs/tasks/TASK-LOG.md` | Scoped maintenance/engineering work not tied to a feature or bug — `TASK-###` |
| `docs/deep-dives/` | Persisted findings from code investigations, one file per question |
| `plans/` | Output: one plan file per feature or bug fix, each citing the IDs above |

Read the requirement and constraint docs **in full** before drafting or revising a
plan — don't rely on what they said earlier in the session, they may have changed.
Also check `docs/decisions/DECISIONS.md` for any accepted ADR the design would
contradict (flag it the way you'd flag an IC tension), and look in `docs/deep-dives/`
for an existing note on the area before investigating it again.

- Ignore `_EXAMPLE_` / `*-EXAMPLE` placeholder entries — they show the format; they
  are not requirements.
- Don't plan against a `Deprecated` entry; follow `Superseded by` to its replacement.
  Call out any `Proposed` entry as not yet confirmed rather than treating it as settled.

## ID conventions

- IDs are `PREFIX-###`, zero-padded to 3 digits, assigned sequentially, **never reused
  or renumbered**. If a requirement is dropped, mark it `Status: Superseded by FR-0XX`
  or `Status: Deprecated` — don't delete the row. History is part of why traceability
  works.
- Prefixes: `FR` functional requirement, `NFR` non-functional requirement, `UC` use
  case, `IC` implementation constraint, `ADR` architecture/design decision, `BUG` a
  logged deviation from intended behavior, `TASK` a scoped maintenance/engineering
  task that is neither new capability nor a correction.
- Every plan phase or major step should cite the ID(s) it satisfies, e.g. `(FR-012,
  UC-003)`. A step with no citation is either scope creep or an uncaptured
  requirement — flag it, don't quietly do it.
- `BUG-###` is a different kind of ID from the rest: it doesn't describe intended
  behavior, it describes a *violation* of it (against an existing FR/NFR/UC where one
  exists, or against an undocumented assumption when it doesn't). Don't force a bug
  into a feature plan's shape — see `plans/_template-bugfix.md`.
- `TASK-###` is a third kind of ID: neither new capability like an FR/NFR/UC nor a
  correction like a BUG — scoped engineering work (dependency bumps, removing dead
  code or stale compile-time directives, retrofitting tests onto code that predates
  them) that doesn't need requirement backing to be legitimate. Don't force it into
  a feature or bugfix plan's shape — see `plans/_template-task.md`.

## The core discipline: don't invent

- If you can't find a requirement backing a feature request, say so and offer to run
  `/new-requirement` rather than writing a plausible-sounding requirement yourself.
  A plan built on an invented requirement looks identical to one built on a real one
  until it's too late to matter.
- If a field is unknown during an interview (priority, target metric, actor), leave
  it as `TBD` rather than guessing. A gap is honest; a plausible guess reads as fact
  later.
- If a proposed plan would violate an implementation constraint (`IC-###`), **say so
  explicitly** and ask how to proceed. Don't silently comply with the constraint by
  redesigning around it without flagging the tension, and don't silently ignore it.

## Filling in templates and logs

When you create a plan or log entry from a template: set `Status` to the single
starting value (`Draft` for plans, `Open` for bug and task log entries) — never copy the `A | B | C`
option list; set `Created` / dates to today's actual date (run `date +%F` if you
don't have it — never guess); and leave genuinely unknown fields `TBD`.

## Custom commands available in this repo

| Command | Does |
|---|---|
| `/init-planning` | First-time setup: interviews you and seeds the requirement docs for an existing or new codebase |
| `/plan-feature <name or description>` | Reads the requirement docs, drafts a phased implementation plan in `plans/`, citing IDs throughout |
| `/fix-bug <description or BUG-###>` | Logs a bug (or resumes one), investigates root cause read-only, drafts a bugfix plan |
| `/deep-dive <area or question>` | Read-only investigation of existing code; checks it against the requirement docs and saves findings |
| `/new-requirement` | Interviews you to add a new FR / NFR / UC / IC entry with the next sequential ID |
| `/plan-task <description or TASK-###>` | Logs a maintenance/engineering task (or resumes one) and drafts a plan for it |
| `/trace <ID>` | Reports everywhere an ID is referenced — docs, plans, code — and flags orphaned requirements |
| `/audit-plan <plan file>` | Checks a plan's ID citations against the requirement docs; reports gaps and coverage, changes nothing |

The `requirements-reviewer` subagent (`.claude/agents/`) reviews the requirement and
constraint docs for vague, untestable, conflicting, or dangling entries. Use it when
asked to review or tighten those docs — it reports findings and never edits.

## Conventions — Arduino / PlatformIO (C/C++)

**Stack:** C++14 (verify each target toolchain actually builds it — older AVR
toolchains default to gnu++11 — and bump to C++17 only if every target board supports
it; check before assuming), Arduino framework via PlatformIO. Target board(s)/MCU family:
`SAMD21`.

### Project layout & `platformio.ini`

- Pin exact versions: `platform = espressif32@6.x.x`, not a bare `platform =
  espressif32`. An unpinned platform/toolchain update is a silent way to break a build
  that worked yesterday.
- One `[env:...]` per physical target, plus a `[env:native]` (`platform = native`, no
  `board =`; uses the host compiler) for anything hardware-independent — this is what makes off-device unit
  testing possible at all.
- Build flags: `-Wall -Wextra` always on; move to `-Werror` once the codebase is
  warning-clean, and don't silence a warning with a pragma without a comment saying
  why.
- Keep algorithmic/business logic in plain C++ (`lib/` or `src/`) that does **not**
  include `Arduino.h`. Confine `Arduino.h`, `digitalRead/Write`, `Serial`, `Wire`,
  `SPI`, etc. to a thin hardware-adapter layer. Logic that doesn't touch a register
  directly has no business including the Arduino core.

### Language & style

- No exceptions, no RTTI — this matches the Arduino core's default
  `-fno-exceptions -fno-rtti` build. Report failure with status codes / enums, not
  `throw`.
- Fixed-width integer types (`uint8_t`, `int16_t`, `uint32_t`, ...) for anything tied
  to register widths, protocol fields, EEPROM/flash layout, or wire formats. Plain
  `int`/`long` are fine for loop counters and ordinary arithmetic, nowhere else.
- No dynamic allocation after `setup()` on RAM-constrained boards (classic AVR,
  SAMD21, anything in the 2–32 KB RAM range): no `new`, no `malloc`, no `String`
  concatenation in a loop. Heap fragmentation on a device with a few KB of RAM is a
  real, field-reported failure mode, not a style nitpick. Use fixed-size buffers
  (plain arrays, or `std::array` where the toolchain ships the C++ standard library —
  classic AVR's typically doesn't) instead. On boards with a real heap (ESP32, Teensy, etc.) this relaxes,
  but allocation still never happens inside an ISR or a tight timing loop.
- Avoid Arduino `String` in library/logic code — use `char[]` buffers or fixed spans.
  `String` is acceptable in top-level sketch glue code, not in reusable modules.
- Long string literals and lookup/constant tables go in flash (`PROGMEM`, `F()`) on
  AVR targets rather than RAM; skip this where the target has ample RAM and no flash
  pressure.
- Every variable shared between an ISR and the main loop is `volatile`. ISRs do the
  minimum possible work (set a flag, copy a value) and defer everything else to
  `loop()`.
- No blocking `delay()` in library code or anywhere with concurrent responsibilities —
  use `millis()`/`micros()`-driven state machines so multiple timed behaviors can
  coexist without starving each other.
- Pin numbers and board-specific constants are named `constexpr`s in one place (e.g.
  `pins.h`), never magic numbers scattered through `.cpp` files.

### Testing

- Hardware-independent logic gets a unit test under `test/`, using PlatformIO's
  Unity-based runner against `env:native` (`pio test -e native`) — tests run on the
  host, no board required.
- Anything that must touch real silicon to verify gets a fake/mock behind the
  hardware-adapter interface; on-target runs (`pio test -e <board>`) are for
  integration checks, not for exercising every logic branch.
- New logic-layer code isn't done without a native test. A bug fix isn't done without
  a regression test that fails before the fix and passes after.

### Static analysis & formatting

- `pio check` runs clean (cppcheck/clang-tidy backend) before a change is considered
  finished, or a new warning is justified inline with a comment.
- `clang-format` (config committed to the repo) is the formatting source of truth —
  don't hand-format around it.

### Documentation

- Every public function/class in `lib/` gets a one-line comment: what it does, units
  where relevant, and any hardware precondition (e.g. "call after `Wire.begin()`").

## Plan mode

For anything nontrivial, prefer Claude Code's built-in plan mode (`Shift+Tab` twice)
combined with `/plan-feature` — read the requirements, think through the approach, and
present the plan before touching code. Read-only research and planning shouldn't need
write access to the codebase.
