<!-- CONTRACT: scoped engineering/maintenance work that isn't new capability (FR/NFR/
     UC) and isn't corrective (BUG) — dependency bumps, removing dead code or stale
     compile-time directives, retrofitting tests onto code that predates them,
     build/tooling changes. The common thread: the "right" outcome is usually
     mechanical and completeness matters (did every instance get handled?), not
     open-ended design. Delete the TASK-EXAMPLE section once you've seen the shape. -->

# Task Log

Convention: `TASK-###`, sequential, never renumbered. Status: `Open` → `Plan Ready` →
`In Progress` → `Done` (or `Won't Do` / `Superseded by TASK-0XX`). **Only the user sets
`Done`** — it's a claim about verified work, not something to self-report from having
written a plan. Category is one of: Dependency
Update, Code Cleanup, Test Coverage, Build/Tooling, Documentation (add categories as
needed).

---

## TASK-EXAMPLE — Add unit tests for the legacy `parser/` module

*(delete this section once you've seen the shape)*

**Category:** Test Coverage
**Status:** Plan Ready
**Created:** YYYY-MM-DD
**Related requirements/constraints:** None found — `parser/` predates FR/NFR/UC
tracking in this repo
**Plan:** `plans/task-parser-test-coverage-plan.md`

**Scope:** Every public function in `parser/`, tested against the fixtures already
in `parser/testdata/`. Does not include refactoring the module itself, even where
the tests reveal awkward internal structure — that's a separate task if pursued.

**Motivation:** `parser/` has zero test coverage and has caused two regressions in
the last quarter that tests would have caught before merge.

---

## TASK-001 — Remove unused `DEBUG`/`DPRINT*` macros from `include/print.h`

**Category:** Code Cleanup
**Status:** Open
**Created:** 2026-10-25
**Related requirements/constraints:** None

**Scope:** `include/print.h` defines `DPRINT`, `DPRINTV`, and `DPRINTF` behind an
`#if DEBUG` guard. `DEBUG` is never defined anywhere in this project's
`platformio.ini` or source, and none of the three macros are referenced anywhere in
`src/` or `include/`. Remove the macros (and the `#if DEBUG`/`#else` scaffolding
around them) from `print.h`. Does not include the rest of `print.h`/`print.cc` (the
`print()`/`flush()` functions), which are in active use.

**Motivation:** Carried over from another project (`nixies-3`, per the commit that
introduced `print.cc`/`print.h`); dead conditional-compilation scaffolding that
nothing in this codebase turns on, discovered while writing this repo's initial
requirement docs.

**Work Log**

**2026-10-25 13:07** — "Lets make those bugs." (clarified to TASK-### after flagging
that these are dead-code cleanup items, not incorrect behavior)
Logged as a task rather than a bug per this repo's own convention (`BUG-###` is for
deviations from intended behavior; dead/unused code is `TASK-###`). The user
confirmed TASK-### over the original "bug" framing when asked.

---

## TASK-002 — Resolve the unfinished `frac()` macro in `include/print.h`

**Category:** Code Cleanup
**Status:** In Progress
**Created:** 2026-10-25
**Related requirements/constraints:** None

**Scope:** `include/print.h` defines `frac(f)` with a `// TODO needs work` comment
directly above it, and a second, commented-out version of the same macro sits right
above that. `frac()` is not referenced anywhere else in `src/` or `include/`. Either
finish it and put it to use, or delete both the live macro and the commented-out
alternative.

**Motivation:** An unused macro with an open `TODO` is easy to mistake for active
functionality; discovered while writing this repo's initial requirement docs.

**Work Log**

**2026-10-25 13:07** — "Lets make those bugs." (clarified to TASK-### after flagging
that these are dead-code cleanup items, not incorrect behavior)
Logged as a task rather than a bug per this repo's own convention (`BUG-###` is for
deviations from intended behavior; dead/unused code is `TASK-###`). The user
confirmed TASK-### over the original "bug" framing when asked.

**2026-10-25 13:30** — "Work on TASKs 001 through 004. Put each on it's own branch
- if reasonable (no conflicts) - work each in isolation."
Deleted the live `frac(f)` macro and the commented-out alternative above it, on
branch `jhrg/task-002-resolve-frac-macro`. Chose deletion over finishing it: it has
no caller anywhere in the codebase, and completing it would mean inventing a use for
it that wasn't requested. `pio run -e adafruit_feather_m0` still succeeds with
identical RAM/flash usage (9736 B / 72400 B). Status left at `In Progress` — only
the user sets `Done`.

---

## TASK-003 — Remove or wire up the `VERSION` build flag

**Category:** Code Cleanup
**Status:** Open
**Created:** 2026-10-25
**Related requirements/constraints:** None

**Scope:** `platformio.ini` sets `-D VERSION=1.5`, but no source file in `src/` or
`include/` references the `VERSION` macro (no printed version string, no `#ifdef`
branch on it). Either remove the build flag, or wire it into `setup()`'s startup
banner the way a version flag would normally be used.

**Motivation:** A build flag with no effect on the compiled firmware is misleading —
it reads as configuration that matters when it currently does nothing; discovered
while writing this repo's initial requirement docs.

**Work Log**

**2026-10-25 13:07** — "Lets make those bugs." (clarified to TASK-### after flagging
that these are dead-code cleanup items, not incorrect behavior)
Logged as a task rather than a bug per this repo's own convention (`BUG-###` is for
deviations from intended behavior; dead/unused code is `TASK-###`). The user
confirmed TASK-### over the original "bug" framing when asked.

---

## TASK-004 — De-duplicate the `DATA_LINE_CHARS` definition

**Category:** Code Cleanup
**Status:** Open
**Created:** 2026-10-25
**Related requirements/constraints:** NFR-006

**Scope:** `DATA_LINE_CHARS` is defined as `161` in both `include/TFTDisplay.h:8`
and again in `src/TFTDisplay.cc:93`, even though `TFTDisplay.cc` includes
`TFTDisplay.h`. Remove the redundant redefinition in `TFTDisplay.cc` so the constant
has one source of truth, and add a one-line comment on the remaining definition
saying why `161` (display line width in the font/size `TFTDisplay.cc` draws with).

**Motivation:** A silently duplicated constant is a latent bug waiting to happen —
if the two definitions are ever changed independently they'd silently disagree, and
there's currently no documented reason for the specific value of `161`; discovered
while writing this repo's initial requirement docs.

**Work Log**

**2026-10-25 13:07** — "Lets make those bugs." (clarified to TASK-### after flagging
that these are dead-code cleanup items, not incorrect behavior)
Logged as a task rather than a bug per this repo's own convention (`BUG-###` is for
deviations from intended behavior; dead/unused code is `TASK-###`). The user
confirmed TASK-### over the original "bug" framing when asked.

---

<!-- Add new tasks via /plan-task, or by hand — keep the section format: metadata
     lines, then Scope / Motivation. Unlike a bug, a task doesn't need a symptom or
     reproduction — it needs a clear boundary of what's included and what isn't,
     since "remove all the X" quietly becoming "remove most of the X" is the
     characteristic failure mode here. -->
